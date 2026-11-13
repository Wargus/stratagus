((nil
  (agent-shell-nono-extra-grants ("/dev/dri" . "read")
                                 ("/sys/devices/" . "read")
                                 ("/dev/udmabuf" . "readwrite")
                                 ("/home/tim/.config/spectaclerc" . "read"))))
