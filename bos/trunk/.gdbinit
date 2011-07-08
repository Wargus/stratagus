python import os
python sys.path.append(os.path.join(os.getcwd(), "tools", "gdb"))
python import boswars_printers, libstdcxx_printers, sdl_printers
python boswars_printers.register_printers(gdb)
python libstdcxx_printers.register_printers(gdb)
python sdl_printers.register_printers(gdb)
