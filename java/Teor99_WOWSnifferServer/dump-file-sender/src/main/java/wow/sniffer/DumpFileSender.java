package wow.sniffer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

public class DumpFileSender {
    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("give filepath as first parameter");
            return;
        }

        try (Socket socket = new Socket()) {
            SocketAddress socketAddress = new InetSocketAddress("127.0.0.1", 6666);
            System.out.println("Try connect to: " + socketAddress);

            socket.connect(socketAddress, 1000);
            System.out.println("Connected success");
            try (BufferedInputStream file = new BufferedInputStream(new FileInputStream(args[0]))) {
                socket.getOutputStream().write(file.readAllBytes());
            }

            BufferedReader console = new BufferedReader(new InputStreamReader(System.in));
            console.readLine();
        }
    }
}
