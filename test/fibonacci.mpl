var t1 : int := 0;
var t2 : int := 1;
var t3 : int := t1 + t2;
var nTimes : int := 0;
print "nth term? ";
read nTimes;
var x : int;
for x in 2..nTimes-1 do
  t1:=t2;
  t2:=t3;
  t3:=t1+t2;
end for;
print t3;
print "\n";
