import marshal, json


data = {12:'twelve', 'feep':list('ciao'), 1.23:4+5, 'HELLO':u'wer'}
with open(f'./test.dat', 'wb') as f:
    marshal.dump(data, f)

with open(f'./test.json', 'w') as f:
    json.dump(data, f)

with open(f'./test.dat', 'rb') as f:
    a = marshal.load(f)
    print(a)


