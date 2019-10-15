
        AREA    MyBinFile1_Section, DATA, READONLY
        EXPORT  MyBinaryImage1

; Includes the binary file MyBinFile1.bin from the current source folder 
MyBinaryImage1
        INCBIN  MyBinFile1.bin
MyBinaryImage1_End

; Use a relative or absolute path to other folders if necessary
;       INCBIN  c:\project\MyBinFile1.bin
; Add further binary files to merge them if necessary
;       INCBIN  MyBinFile2.bin

; define a constant which contains the size of the image above
MyBinaryImage1_length
        DCD     MyBinaryImage1_End - MyBinaryImage1

        EXPORT  MyBinaryImage1_length

        END
