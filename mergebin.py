import os

def mergeBinProccess( files, fileSaveName):
    bin = b''
    startAddrLast = files[0][1]
    fileSizeLast  = 0
    for file, addr in files:
        fillLen = addr - (startAddrLast + fileSizeLast)
        if fillLen > 0:               # fill 0xFF
             fill = bytearray([0xFF for i in range(fillLen)])
             bin += fill
        with open(file, "rb") as f:   # add bin file content
             bin += f.read()
        startAddrLast = addr
        fileSizeLast = os.path.getsize(file)
    with open(fileSaveName, "wb") as f:
         f.write(bin)
         
if __name__ == '__main__':
    file1 = r'./build/XiUOS_kd233_kernel.bin'
    file1_start_addr = 0
    file2 = r'./build/XiUOS_kd233_app.bin'
    file2_start_addr = 1024 * 1024 + 4096
    newfile = r'./build/XiUOS_kd233.bin'

    file = [ [file1 , file1_start_addr] , [file2 , file2_start_addr]  ]
    mergeBinProccess(file, newfile)