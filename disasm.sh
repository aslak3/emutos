VMA=0x0000000000800000
IMG=ddraig68k.img
MAP=ddraig68k/ddraig68k.map
/opt/cross-mint/bin/m68k-atari-mint-objdump --target=binary --architecture=m68k --adjust-vma=$VMA -D $IMG \
          | sed -e '/^ *[0-9a-f]*:/!d;s/^    /0000/;s/^   /000/;s/^  /00/;s/^ /0/;s/:   /: /' > dsm_temp_code.txt
sed -e '/^ *0x/!d;s///;s/  */:  /;s/^00000000//;/^00000001:  ASSERT /d;/ \. = /d;s/ = .*//' $MAP > dsm_temp_labels.txt
cat dsm_temp_code.txt dsm_temp_labels.txt | LC_ALL=C sort > dsm.txt
rm dsm_temp*.txt