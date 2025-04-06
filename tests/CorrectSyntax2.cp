def int gcd(int a, int b)
  int x, y;
  double z;
  
  a = gcd(1, gcd(gcd(1, 2), 3));
  x = 1;
  y = 2;
  z = 10.1;

  if (a==b) then 
    return (a)
  fi; 
  
  if (gcd(1,2)>1.2) then
    return(gcd(1, gcd(gcd(1, 2), 3)))
  else 
    return(gcd(a,b-a))
  fi; 
fed; 

print gcd(21,15);
print 45; 
print 2*(gcd(21, 28) + 6).