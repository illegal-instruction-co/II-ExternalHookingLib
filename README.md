# II-ExternalHookingLib
 External Hooking ( Bypasss process byte patching checks | Injector included )

Preface:

Yes, friends, as I mentioned below, I had a problem while doing research on SetWindowsHookExW today.

External hooking ( target internal byte patching protection )

I wonder if there is a way to throw an external hook on processes with bypassing internal byte checks? Or am I just being silly?

As you can see in the attachment below, the injected dll has the same linear address in the internal process and the target process:
![process hacker](https://prnt.sc/23ahe9p)
Bingo!

So all that needs to be done is to innocently inject the DLL (without using the CRT) and write a few bytes from the outside to the address we want it hooked to!

# More details: 
### https://www.unknowncheats.me/forum/anti-cheat-bypass/481968-external-hooking-bypasss-process-byte-patching-checks-injector-included.html#post3321216
### https://www.unknowncheats.me/forum/anti-cheat-bypass/481933-external-hooking-target-internal-byte-patching-protection.html?posted=1#post3321150
