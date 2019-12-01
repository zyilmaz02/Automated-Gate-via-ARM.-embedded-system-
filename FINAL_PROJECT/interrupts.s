        AREA |.text|, CODE, READONLY
		EXPORT DisableInterrupts
        EXPORT EnableInterrupts 
;*********** DisableInterrupts *************** 

DisableInterrupts  CPSID  I     ;set I=1
                   BX     LR

;*********** EnableInterrupts ***************

EnableInterrupts   CPSIE  I    ;set I=0

                   BX     LR
		
		END