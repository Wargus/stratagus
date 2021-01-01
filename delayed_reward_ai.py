import sys
import atexit
import socket
import struct
import random
import torch
from torch import nn
import numpy as np
from pathlib import Path
from collections import deque
import random, datetime, os, copy
import numpy as np
import time, datetime
import matplotlib.pyplot as plt


class AI:
    def __init__(self, state_dim, action_dim, save_dir, load_file=None):
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.save_dir = save_dir

        self.use_cuda = torch.cuda.is_available()

        # Stratagus's DNN to predict the most optimal action - we implement this in the Learn section
        self.net = StratagusNet(self.state_dim, self.action_dim).float()

        if self.use_cuda:
            self.net = self.net.to(device="cuda")

        self.exploration_rate = 1
        self.exploration_rate_decay = 0.99999975
        self.exploration_rate_min = 0.1
        self.curr_step = 0

        self.save_every = 5e4  # no. of experiences between saving the net

        self.memory = deque(maxlen=100000)
        self.batch_size = 64

        self.gamma = 0.9
        self.optimizer = torch.optim.Adam(self.net.parameters(), lr=0.00025)
        # self.optimizer = torch.optim.Adam(self.net.parameters(), lr=0.1) # faster learning rate
        self.loss_fn = torch.nn.SmoothL1Loss()

        self.burnin = 5e4  # min. experiences before training
        self.learn_every = 32  # no. of experiences between updates to Q_online
        self.sync_every = 1e4  # no. of experiences between Q_target & Q_online sync

        if load_file:
            checkpoint = torch.load(load_file)
            self.net.load_state_dict(checkpoint['net_state_dict'])
            self.optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
            self.exploration_rate = checkpoint['exploration_rate']

    def act(self, state):
        """
        Given a state, choose an epsilon-greedy action and update value of step.
        Inputs:
        state(LazyFrame): A single observation of the current state, dimension is (state_dim)
        Outputs:
        action_idx (int): An integer representing which action Stratagus will perform
        """
        # EXPLORE
        if np.random.rand() < self.exploration_rate:
            action_idx = np.random.randint(self.action_dim)

        # EXPLOIT
        else:
            state = state.__array__()
            if self.use_cuda:
                state = torch.tensor(state, dtype=torch.float).cuda()
            else:
                state = torch.tensor(state, dtype=torch.float)
            state = state.unsqueeze(0)
            action_values = self.net(state, model="online")
            action_idx = torch.argmax(action_values, axis=1).item()

        # decrease exploration_rate
        self.exploration_rate *= self.exploration_rate_decay
        self.exploration_rate = max(self.exploration_rate_min, self.exploration_rate)

        # increment step
        self.curr_step += 1
        return action_idx

    def cache(self, state, next_state, action, reward, done):
        """
        Store the experience to self.memory (replay buffer)

        Inputs:
        state (LazyFrame),
        next_state (LazyFrame),
        action (int),
        reward (float),
        done(bool))
        """
        state = state.__array__()
        next_state = next_state.__array__()

        if self.use_cuda:
            state = torch.tensor(state, dtype=torch.float).cuda()
            next_state = torch.tensor(next_state, dtype=torch.float).cuda()
            action = torch.tensor([action]).cuda()
            reward = torch.tensor([reward]).cuda()
            done = torch.tensor([done]).cuda()
        else:
            state = torch.tensor(state, dtype=torch.float)
            next_state = torch.tensor(next_state, dtype=torch.float)
            action = torch.tensor([action])
            reward = torch.tensor([reward])
            done = torch.tensor([done])

        self.memory.append((state, next_state, action, reward, done,))

    def recall(self):
        """
        Retrieve a batch of experiences from memory
        """
        batch = random.sample(self.memory, self.batch_size)
        state, next_state, action, reward, done = map(torch.stack, zip(*batch))
        return state, next_state, action.squeeze(), reward.squeeze(), done.squeeze()

    def learn(self):
        """Update online action value (Q) function with a batch of experiences"""
        if self.curr_step % self.sync_every == 0:
            self.sync_Q_target()

        if self.curr_step % self.save_every == 0:
            self.save()

        if self.curr_step < self.burnin:
            return None, None

        if self.curr_step % self.learn_every != 0:
            return None, None

        # Sample from memory
        state, next_state, action, reward, done = self.recall()

        # Get TD Estimate
        td_est = self.td_estimate(state, action)

        # Get TD Target
        td_tgt = self.td_target(reward, next_state, done)

        # Backpropagate loss through Q_online
        loss = self.update_Q_online(td_est, td_tgt)

        return (td_est.mean().item(), loss)

    def td_estimate(self, state, action):
        current_Q = self.net(state, model="online")[
            np.arange(0, self.batch_size), action
        ]  # Q_online(s,a)
        return current_Q

    @torch.no_grad()
    def td_target(self, reward, next_state, done):
        next_state_Q = self.net(next_state, model="online")
        best_action = torch.argmax(next_state_Q, axis=1)
        next_Q = self.net(next_state, model="target")[
            np.arange(0, self.batch_size), best_action
        ]
        return (reward + (1 - done.float()) * self.gamma * next_Q).float()

    def update_Q_online(self, td_estimate, td_target):
        loss = self.loss_fn(td_estimate, td_target)
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()
        return loss.item()

    def sync_Q_target(self):
        self.net.target.load_state_dict(self.net.online.state_dict())

    def save(self):
        save_path = (
            self.save_dir / f"stratagus_net_{int(self.curr_step // self.save_every)}.chkpt"
        )
        torch.save({
            "net_state_dict": self.net.state_dict(),
            "optimizer_state_dict": self.optimizer.state_dict(),
            "exploration_rate": self.exploration_rate,
        }, save_path)
        print(f"StratagusNet saved to {save_path} at step {self.curr_step}")


class StratagusNet(nn.Module):
    def __init__(self, input_dim, output_dim):
        super().__init__()
        c = input_dim

        self.l1 = nn.Linear(input_dim, 512)
        self.l2 = nn.Linear(512, 1024)
        self.l3 = nn.Linear(1024, 256)
        self.l4 = nn.Linear(256, output_dim)

        self.online = torch.nn.Sequential(
            self.l1,
            nn.ReLU(),
            self.l2,
            nn.ReLU(),
            self.l3,
            nn.Dropout(p=0.6),
            nn.ReLU(),
            self.l4,
            nn.Softmax(dim=-1)
        )

        self.target = copy.deepcopy(self.online)

        # Q_target parameters are frozen.
        for p in self.target.parameters():
            p.requires_grad = False

    def forward(self, input, model):
        if model == "online":
            return self.online(input)
        elif model == "target":
            return self.target(input)



class MetricLogger:
    def __init__(self, save_dir):
        self.save_log = save_dir / "log"
        with open(self.save_log, "w") as f:
            f.write(
                f"{'Episode':>8}{'Step':>8}{'Epsilon':>10}{'MeanReward':>15}"
                f"{'MeanLength':>15}{'MeanLoss':>15}{'MeanQValue':>15}"
                f"{'TimeDelta':>15}{'Time':>20}\n"
            )
        self.ep_rewards_plot = save_dir / "reward_plot.jpg"
        self.ep_lengths_plot = save_dir / "length_plot.jpg"
        self.ep_avg_losses_plot = save_dir / "loss_plot.jpg"
        self.ep_avg_qs_plot = save_dir / "q_plot.jpg"

        # History metrics
        self.ep_rewards = []
        self.ep_lengths = []
        self.ep_avg_losses = []
        self.ep_avg_qs = []

        # Moving averages, added for every call to record()
        self.moving_avg_ep_rewards = []
        self.moving_avg_ep_lengths = []
        self.moving_avg_ep_avg_losses = []
        self.moving_avg_ep_avg_qs = []

        # Current episode metric
        self.init_episode()

        # Timing
        self.record_time = time.time()

    def log_step(self, reward, loss, q):
        self.curr_ep_reward += reward
        self.curr_ep_length += 1
        if loss:
            self.curr_ep_loss += loss
            self.curr_ep_q += q
            self.curr_ep_loss_length += 1

    def log_episode(self):
        "Mark end of episode"
        self.ep_rewards.append(self.curr_ep_reward)
        self.ep_lengths.append(self.curr_ep_length)
        if self.curr_ep_loss_length == 0:
            ep_avg_loss = 0
            ep_avg_q = 0
        else:
            ep_avg_loss = np.round(self.curr_ep_loss / self.curr_ep_loss_length, 5)
            ep_avg_q = np.round(self.curr_ep_q / self.curr_ep_loss_length, 5)
        self.ep_avg_losses.append(ep_avg_loss)
        self.ep_avg_qs.append(ep_avg_q)

        self.init_episode()

    def init_episode(self):
        self.curr_ep_reward = 0.0
        self.curr_ep_length = 0
        self.curr_ep_loss = 0.0
        self.curr_ep_q = 0.0
        self.curr_ep_loss_length = 0

    def record(self, episode, epsilon, step):
        mean_ep_reward = np.round(np.mean(self.ep_rewards[-100:]), 3)
        mean_ep_length = np.round(np.mean(self.ep_lengths[-100:]), 3)
        mean_ep_loss = np.round(np.mean(self.ep_avg_losses[-100:]), 3)
        mean_ep_q = np.round(np.mean(self.ep_avg_qs[-100:]), 3)
        self.moving_avg_ep_rewards.append(mean_ep_reward)
        self.moving_avg_ep_lengths.append(mean_ep_length)
        self.moving_avg_ep_avg_losses.append(mean_ep_loss)
        self.moving_avg_ep_avg_qs.append(mean_ep_q)

        last_record_time = self.record_time
        self.record_time = time.time()
        time_since_last_record = np.round(self.record_time - last_record_time, 3)

        print(
            f"Episode {episode} - "
            f"Step {step} - "
            f"Epsilon {epsilon} - "
            f"Mean Reward {mean_ep_reward} - "
            f"Mean Length {mean_ep_length} - "
            f"Mean Loss {mean_ep_loss} - "
            f"Mean Q Value {mean_ep_q} - "
            f"Time Delta {time_since_last_record} - "
            f"Time {datetime.datetime.now().strftime('%Y-%m-%dT%H:%M:%S')}"
        )

        with open(self.save_log, "a") as f:
            f.write(
                f"{episode:8d}{step:8d}{epsilon:10.3f}"
                f"{mean_ep_reward:15.3f}{mean_ep_length:15.3f}{mean_ep_loss:15.3f}{mean_ep_q:15.3f}"
                f"{time_since_last_record:15.3f}"
                f"{datetime.datetime.now().strftime('%Y-%m-%dT%H:%M:%S'):>20}\n"
            )

        for metric in ["ep_rewards", "ep_lengths", "ep_avg_losses", "ep_avg_qs"]:
            plt.plot(getattr(self, f"moving_avg_{metric}"))
            plt.savefig(getattr(self, f"{metric}_plot"))
            plt.clf()


if __name__ == "__main__":
    # Listen for incoming datagrams
    localIP = "127.0.0.1"
    localPort = int(sys.argv[1])
    buffersize = 1024
    sock = socket.socket(family=socket.AF_INET)
    sock.bind((localIP, localPort))
    atexit.register(lambda: sock.close())
    sock.listen(5)

    if len(sys.argv) > 2:
        load_file = sys.argv[2]
    else:
        load_file = None

    use_cuda = torch.cuda.is_available()
    print(f"Using CUDA: {use_cuda}")
    print()

    save_dir = Path("checkpoints") / datetime.datetime.now().strftime("%Y-%m-%dT%H-%M-%S")
    save_dir.mkdir(parents=True)
    stratagus = None
    logger = MetricLogger(save_dir)
    num_state = 0
    num_actions = 0
    e = 0

    while True:
        print("TCP server up and listening on", localIP, localPort)
        (clientsocket, address) = sock.accept()
        print("connection", address)
        long_size = struct.calcsize('!l')
        last_state = None
        last_action = 0

        while(True):
            command = clientsocket.recv(1)
            if not command:
                break
            if command == b"I":
                states = ord(clientsocket.recv(1))
                actions = ord(clientsocket.recv(1))
                if stratagus is not None:
                    assert states == num_state, ("%d:%d" % (states, num_state))
                    assert actions == num_actions, ("%d:%d" % (actions, num_actions))
                else:
                    num_state, num_actions = states, actions
                    state_unpack_fmt = "!" + "l" * num_state
                    stratagus = AI(state_dim=num_state, action_dim=num_actions, save_dir=save_dir, load_file=load_file)
            elif command == b"S" or command == b"E":
                r = b""
                while len(r) < long_size:
                    r += clientsocket.recv(long_size - len(r))
                reward = struct.unpack("!l", r)[0]
                r = b""
                expected = long_size * num_state
                while len(r) < expected:
                    r += clientsocket.recv(expected - len(r))
                args = struct.unpack(state_unpack_fmt, r)
                state = np.array(args, dtype=np.float)

                if last_state is not None:
                    # Remember
                    stratagus.cache(last_state, state, last_action, reward, False)
                    # Learn
                    q, loss = stratagus.learn()
                    # Logging
                    logger.log_step(reward, loss, q)

                if command == b"S":
                    # Run agent on the state
                    last_action = stratagus.act(state)
                    last_state = state
                    # action is performed
                    clientsocket.sendall(bytearray([last_action]))
                else:
                    assert command == b"E"
                    logger.log_episode()
                    logger.record(episode=e, epsilon=stratagus.exploration_rate, step=stratagus.curr_step)
                    e += 1
