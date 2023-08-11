import pexpect

command = '/path/to/wallet-api --rpc-password carlxvigustaf'
prompt = 'Type exit to save and shutdown.'

child = pexpect.spawn(command, encoding='utf-8')

try:
    child.expect(prompt)
    child.sendline('exit')
    child.expect(pexpect.EOF)
except pexpect.exceptions.EOF:
    pass

child.close()
