echo $@
foo=("cclient c1 unix4 $@")
for i in {2..300}
do
   foo=("${foo[@]}" "cclient Test_$i unix4 $@")
done


for cmd in "${foo[@]}"; do {
  echo "Process \"$cmd\" started";
  $cmd & pid=$!
  # sleep .1
  PID_LIST+=" $pid";
} done
# sleep 10

# foo=("cclient d1 unix4 $@")
# for i in {2..200}
# do
#    foo=("${foo[@]}" "cclient d$i unix4 $@")
# done


# for cmd in "${foo[@]}"; do {
#   echo "Process \"$cmd\" started";
#   $cmd & pid=$!
#   sleep .1
#   PID_LIST+=" $pid";
# } done
trap "kill $PID_LIST" SIGINT

echo "Parallel processes have started";

wait $PID_LIST



echo "All processes have completed";
