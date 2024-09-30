package wow.sniffer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;
import wow.sniffer.net.Packet;
import wow.sniffer.net.PacketReader;

import java.io.EOFException;
import java.net.Inet4Address;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

@SpringBootApplication
public class Server implements CommandLineRunner {

    private final Logger log = LoggerFactory.getLogger(Server.class);

    @Autowired
    private ApplicationContext ctx;

    public static void main(String[] args) {
        SpringApplication.run(Server.class, args);
    }

    @Override
    public void run(String... args) throws Exception {
        try (ServerSocket serverSocket = new ServerSocket(6666, 0, Inet4Address.getByName("127.0.0.1"))) {
            while (true) {
                log.info("Server start listen on: " + serverSocket.getLocalSocketAddress());
                try (Socket clientSocket = serverSocket.accept()) {
                    log.info("New connection from: " + clientSocket.getRemoteSocketAddress());
                    BlockingQueue<Packet> queue = new LinkedBlockingQueue<>();
                    PacketHandler packetHandler = ctx.getBean(PacketHandler.class);
                    packetHandler.setName("PacketHandler");
                    packetHandler.setQueue(queue);
                    packetHandler.start();
                    try (PacketReader packetReader = new PacketReader(clientSocket.getInputStream())) {
                        while (true) {
                            Packet packet = packetReader.readPacket();
                            queue.add(packet);
                        }
                    } catch (EOFException e) {
                        log.info("Close connection from: " + clientSocket.getRemoteSocketAddress());
                        packetHandler.interrupt();
                        log.info("Wait while PacketHandler thread stop");
                        packetHandler.join();
                        log.info("PacketHandler thread stopped");
                    }
                }
            }
        }
    }
}
