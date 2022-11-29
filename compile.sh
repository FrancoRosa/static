# st-flash write generic_boot20_pc13.bin 0x08000000
start_time=$(date +%s.%3N)

echo ''
echo '... compile'
echo ''

arduino-cli compile --fqbn stm32duino:STM32F1:genericSTM32F103C6:upload_method=STLinkMethod,cpu_speed=speed_48mhz,opt=osstd static.ino --build-path build 

SUCCESS=$?
RESPONSE=0

# if [ "$SUCCESS" == "$RESPONSE" ]
# then
#   compile_time=$(date +%s.%3N)
#   elapsed=$(echo "scale=3; $compile_time - $start_time" | bc)
#   echo '... compiled, elapsed time:' $elapsed 'ms'

  
  # echo ''
  # echo '... upload  '
  # echo ''

  # sudo st-flash --hot-plug write build/static.ino.bin 0x08000000 
  # sudo st-flash reset 

  # end_time=$(date +%s.%3N)
  # elapsed=$(echo "scale=3; $end_time - $start_time" | bc)
  # echo ''
  # echo '... done, elapsed time:' $elapsed 'ms'
  # echo ''
# else
#   compile_time=$(date +%s.%3N)
#   elapsed=$(echo "scale=3; $compile_time - $start_time" | bc)
#   echo '... compiled, elapsed time:' $elapsed 'ms'

#   echo ''
#   echo '... compile failed  '
#   echo ''
# fi
