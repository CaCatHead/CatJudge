import java.io.*;

public class Main {
    public static void main(String args[]) {
        try {
            File file = new File("./ans.txt");
            FileReader fr = new FileReader(file);
            BufferedReader br = new BufferedReader(fr);
            StringBuffer sb = new StringBuffer();
            String line;
            while ((line = br.readLine()) != null) {
                sb.append(line);
                sb.append("\n");
            }
            fr.close();
            System.out.print(sb.toString());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
