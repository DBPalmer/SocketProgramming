A) Lucas (Deuce) Palmer

B) 5848992575

C) I have completed the entire project, including the admin extra credit

D)  client.cpp: client
    serverM.cpp: main server
    serverS.cpp: backend science server
    serverL.cpp: backend literature server
    serverH.cpp: backend history server

E)  UDP Setup (message were used to check everything was working properly)
        serverS -> serverM: "serverS"
        serverL -> serverM: "serverL"
        serverH -> serverM: "serverH"
    login
        client -> serverM: "username, password"
        serverM -> client: "success", "fail", or "userfail"
    book code request
        client -> serverM: bookcode, or bookcode! for admin
        serverM -> server S,L,H: bookcode, or bookcode! for admin
        server S,L,H -> serverM: inventory, "available", "notavailable", or "FAIL"
        serverM -> client: inventory, "available", "notavailable", or "FAIL"

G) No idiosyncrasies

H)  Source #1: Beej (https://beej.us/guide/bgnet/html/#inet_ntopman)
        used for TCP and UDP connections
            locations:  CreateAndBind(), Listen(), Accept(), UDPCreateAndBind() of serverM.cpp (along with send()'s, recv()'s, sendto()'s, and recvfrom()'s in main())
                        CreateAndSend() of serverS.cpp (along with sendto()'s and recvfrom()'s in main())
                        CreateAndSend() of serverL.cpp (along with sendto()'s and recvfrom()'s in main())
                        CreateAndSend() of serverH.cpp (along with sendto()'s and recvfrom()'s in main())
                        CreateAndConnect() of client.cpp (along with send()'s and recv()'s in main())
    Source #2: getsockname documentation (https://support.sas.com/documentation/onlinedoc/sasc/doc700/html/lr2/zockname.htm)
        used to learn how to print the dynamic port "(int)ntohs(my_addr.sin_port)"
            location: main() of client.cpp
    Source #3: Project Handout
        getsockname() code
            location: CreateAndConnect() of client.cpp