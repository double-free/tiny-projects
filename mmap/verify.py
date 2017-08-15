#!/usr/bin/env python3
import sys

def read_by_tokens(fileobj):
    for line in fileobj:
        for token in line.split():
            yield token

def verify(txt_file):
    words = []
    with open('words.txt') as f:
        for line in f:
            words.append(line.strip())

    with open(txt_file) as f:
        for word in read_by_tokens(f):
            word = word.strip()
            if word not in words:
                print('word "', word, '" not in ', words)
                return False
        print('File check success')
        return True

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: ", sys.argv[0], "<txt file>")
        exit(0)
    verify(sys.argv[1])
