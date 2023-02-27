import os
import sys
import random



if __name__ == "__main__":
    lst = list(range(0,8))
    random.shuffle(lst)
    with open("file2nums.bin","wb+") as f:
        f.write(len(lst).to_bytes(4,'little'))
        for n in lst:
            f.write(n.to_bytes(4,'little'))
