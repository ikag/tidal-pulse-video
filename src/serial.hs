import Sound.Tidal.Tempo (Tempo, logicalTime, clocked, clockedTick, cps)
import System.Hardware.Serialport
import qualified Data.ByteString.Char8 as B
import Data.Char
import Data.Word

import Sound.OSC.FD
import Sound.OSC.Datum
import System.IO
import Control.Concurrent

tpb = 1

wave n = drop i s ++ take i s
  where s = "¸.·´¯`·.´¯`·.¸¸.·´¯`·.¸<" ++ eye ++ ")))><"
        i = n `mod` (length s)
        eye | n `mod` 4 == 0 = "O"
            | otherwise = "º"

onTick ard current ticks =
    do let message = B.pack [chr $ if (ticks `mod` 4 == 0) then 0xff else 0x00]
       forkIO $ do threadDelay $ floor $ 0.09 * 1000000
                   send ard message
                   return ()
       threadDelay $ floor $ 0.04 * 1000000
       putStr $ "Pulse " ++ (show ticks) ++ " " ++ (wave ticks) ++ "\r"
       hFlush stdout
       return ()

main = do ard <- openSerial "/dev/ttyACM0" defaultSerialSettings {commSpeed = CS9600}
          clockedTick 4 $ onTick ard
