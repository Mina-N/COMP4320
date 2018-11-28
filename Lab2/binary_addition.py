def ab(b1, b2):
    if not (b1 and b2):  # b1 or b2 is empty
        return b1 + b2
    head = ab(b1[:-1], b2[:-1])
    if b1[-1] == '0':  # 0+1 or 0+0
        return head + b2[-1]
    if b2[-1] == '0':  # 1+0
        return head + '1'
    #      V    NOTE   V <<< push overflow 1 to head
    return ab(head, '1') + '0'

def bin_add(*args): return bin(sum(int(x, 2) for x in args))[2:]

def bit_not(n, numbits=8):
    return bin(((1 << numbits) - 1 - n))

def truncateBitString(inverted_new_sum):
    list_inverted_sum = list(inverted_new_sum)
    index = 0
    indexedValue = list_inverted_sum[index]
    while (indexedValue != 'b'):
        index += 1
        indexedValue = list_inverted_sum[index]
    inverted_new_sum = inverted_new_sum[(index + 1):] #TODO: check this!
    if (len(inverted_new_sum) < 8):
        while (len(inverted_new_sum) < 8):
            inverted_new_sum = "0" + inverted_new_sum
    print(inverted_new_sum)
    return inverted_new_sum

def calculateChecksum(binaryString1, binaryString2):
    return_value = ab(binaryString1, binaryString2)
    new_return_value = return_value[1:]
    print(return_value)
    if (len(return_value) > 8):
        new_sum = bin_add(new_return_value, str(1))
        print(new_sum)
        inverted_new_sum = bit_not(int(new_sum,2))
    else:
        inverted_new_sum = bit_not(int(return_value,2))

    print(inverted_new_sum)
    return truncateBitString(inverted_new_sum)
