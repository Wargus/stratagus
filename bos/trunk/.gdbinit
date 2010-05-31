python import os
python sys.path.append(os.path.join(os.getcwd(), "tools"))
python import boswars_printers, libstdcxx_printers
python boswars_printers.register_printers(gdb)
python libstdcxx_printers.register_printers(gdb)
