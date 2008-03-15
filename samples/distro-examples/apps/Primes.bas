'
'	Displays prime numbers
'
const z=1000
dim primes(z)
primes(1)=2
cand=3
i=2
repeat
  testcand
  if prime then primes(i)=cand : i=i+1 : if not (i%100) then ? i
  cand=cand+2
until i>z
cls
for i=1 to z step 5
  ? primes(i),primes(i+1),primes(i+2),primes(i+3),primes(i+4)
if !((i+4)%50) then ? ""+chr$(10)+"Primes "+str$(i+4-49)+" - "+str$(i+4) : pause : cls
next i
stop

sub testcand
s=sqr(cand)
n=1
prime=false
while primes(n)<=s
  t=cand/primes(n)
  if t=int(t) then exit sub
  n=n+1
wend
prime=true
exit sub
end


