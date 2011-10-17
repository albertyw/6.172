for i in range(0,2**8):
    binary = bin(i)[2:]
    binary = '0'*(8-len(binary))+binary
    flip = '0b'+binary[::-1]
    print int(flip, 2)
