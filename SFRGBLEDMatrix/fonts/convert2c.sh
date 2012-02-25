#!/bin/bash

v="$(gawk '
/^#/{
  lc=0
  next
  nchar=0
}
{
  line[lc]=line[lc]$0
  if(lc==0)
    clen[nchar++]=length($0)
  lc++
}
END{
  s=0
  printf "prog_uint16_t coffset_'$1'p[] PROGMEM={"
  for(c=0;c<nchar;c++) {
    printf s", "
    s+=clen[c]
  }
  printf s
  printf "}\\;\\\\n"
  for(c=0;c<'$1';c++) {
    count=int(length(line[c])/8)
    if(length(line[c])%8)
      count++
    printf "prog_uchar line"c"_'$1'p[] PROGMEM={"
    for(d=1;;d+=8) {
      s=substr(line[c], d, 8);
      if(s=="")
        break;
      printf "$(echo -n 0x)$(printf x $((2#"s
      if(length(s)<8)
        for(e=0;e<8-length(s);e++)
          printf 0
      if(--count)
        printf ")))$(echo -n \", \")"
      else
        printf ")))"
    }
    print "}\\;\\\\n"
  }
  print "prog_uchar *line_'$1'p[] PROGMEM={"
  print "line0_'$1'p"
  for(i=1;i<'$1';i++)
    print ", line"i"_'$1'p"
  print "}\\;\\\\n"
}
' < font${1}p | sed -r 's/x/%x/g')"
eval echo -e $v | tr -d % | sed -r 's/^ (.+)/\1/g'
