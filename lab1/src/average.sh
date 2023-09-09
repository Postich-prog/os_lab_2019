echo "Кол-во аргументов " $#
for n in $@
do
  let sum=$sum+$n
done
D=$(bc<<<"scale=3;$sum/$#")
echo "СР арифм : $D"
