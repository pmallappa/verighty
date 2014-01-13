#!/usr/bin/env python
######
# Author: Prem Mallappa <prem.mallappa@gmail.com>
#
# Description: Automation script to allow cavium simulator to U-boot prompt,
#              Later execute some commands in Linux
#              Also allows sending slow and sending in human simulated form.
######

import pexpect, time, sys
import pdb

host='localhost'
port=2020

#conf="CONF_81M"
conf="CONF_70M"

#sim = pexpect.spawn('telnet %s %d' %(host, port))
#sim.setecho(1)

class Telnet(pexpect.spawn):
	def __init__(self, data):
		self.sendtype = 'normal'
		pexpect.spawn.__init__(self, 'telnet %s' %data)

	def sendslow(self, text, wait=0.01, sendnl=True):
		for char in text:
			self.send(char)
			time.sleep(wait)
		if sendnl:
			self.send('\n')

	# Simulate human typing like behaviour
	def sendhuman(self, text, wait=0.1, sendnl=True):
		self.sendslow(text, wait, sendnl)

	# Send commands if the pattern not found in line print the line
	# and continue to expect
	def sendcmds(self, exp, arr=[], timeout=None, wait=0, print_non_match=True, sendtype=None, debug=False):
		eol = '\n'
		# array index
		li = 0
		if sendtype is None:
			sendtype = self.sendtype

		while True:
			index = self.expect([
				'%s\n%s'%(exp,exp),
				'%s\n\n%s'%(exp,exp),
				exp,
				eol	     # Should always be last
				], timeout=timeout)
			if index == 3:
				if print_non_match: # This is hack to display all the non matched lines
					print(self.before)
				continue

			if self.after != eol:
				print(self.after, end='')
			if sendtype is 'normal':
				self.sendline(arr[li])
			elif sendtype is 'human':
				self.sendhuman(arr[li])
			elif sendtype is 'slow':
				self.sendslow(arr[li])

			if wait:
				time.sleep(wait)
			li += 1
			if li == len(arr):
				break

	def setsendtype(self, t):
		self.sendtype = t

	def settimeout(self, t):
		self.timeout = t


t = Telnet('%s %d'%(host, port))

## For debugging
#t.logfile = sys.stdout

ubootcmds_pre = [
'setenv nb \'namedfree __tmp_load\'',
'setenv na3 \'namedalloc lblock1 0x10000000 0x20000000\'',
'setenv mem \'mem=block:lblock0,lkdump,lblock1\'',
'setenv ba \'numcores=1 namedblock=lblock0 endbootargs\'',
'setenv slram \'slram=root,0x40000000,+1073741824 root=1f00\'',
]

ubootcmds_64M = [
'setenv na1 \'namedalloc lblock0 0x03f00000 0x00100000\'',
'setenv na2 \'namedalloc lkdump 0x04000000 0x04000000\'',
'setenv c \'crashkernel=64M@64M\'',
]
ubootcmds_160M = [
'setenv na1 \'namedalloc lblock0 0x09f00000 0x00100000\'',
'setenv na2 \'namedalloc lkdump 0x05100000 0x0a000000\'',
#'setenv c \'crashkernel=81M@160M\'',
]

ubootcmds_post = [
'setenv pkb \'bootoctlinux 0x10000000 $(ba) $(mem) $(c) $(slram)\'',
'setenv pkblk \'$(nb); $(na1); $(na2);$(na3)\'',
'run pkblk',
'run pkb',
]

ubootprompt = 'Octeon simulator# '

if conf == 'CONF_81M':
        ubootcmds_160M.append('setenv c \'crashkernel=81M@160M\'')
elif conf == 'CONF_70M':
        ubootcmds_160M.append ('setenv c \'crashkernel=70M@160M\'')
else:
        pass

t.sendcmds(ubootprompt, ubootcmds_pre)
t.sendcmds(ubootprompt, ubootcmds_160M)
t.sendcmds(ubootprompt, ubootcmds_post)
t.maxread = 1

linuxcmds_pre = [
'sysctl -w kernel.panic_on_oops=1',
]

linuxcmds_64M = [
'/root/kexec -p /root/vmlinux.64.kdump.64M',
]

linuxcmds_160M = [
#'ap=\'mem=block:lkdump console=ttyS0,115200 \'',
'/root/kexec -p /root/vmlinux.64.kdump.160M',
]

linuxcmds_post = [
'insmod /root/panic.ko',
]

## Start python debugger
#pdb.set_trace()

t.settimeout(None)

linuxprompt = '~ # '

t.sendcmds(linuxprompt, linuxcmds_pre, timeout=None)
#sendcmds $linuxprompt linuxcmds_64M, timeout=None)
#pdb.set_trace()
t.sendcmds(linuxprompt, linuxcmds_160M, timeout=None)

i = t.expect([linuxprompt, 'Inconsistency detected by ld.so'], timeout=None)
if i == 0:
        t.sendcmds(linuxprompt, linuxcmds_post, timeout=None)
        t.expect(linuxprompt, timeout=None)
        t.close()
elif i == 1:
        print(t.before)
        print(t.after)
        t.close()
else:
        pass
