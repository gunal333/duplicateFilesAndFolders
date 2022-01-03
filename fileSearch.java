import java.util.ArrayList;

public class fileSearch{
    static {
        System.loadLibrary("fileSearch");
        System.loadLibrary("dirSearch");
    }
    private native static ArrayList<String> getDuplicateFiles(String fileName);
    private native static ArrayList<String> getDuplicateDir(String dirName);
    public static void main(String[] args) {
        String path = "D:/JNIExample/systeminfo.txt";
        String dirName = "D:/logRecordsFetcher/";
        System.out.println("Searching duplicates for : "+path+"  ...");
        ArrayList<String> duplicateFiles = getDuplicateFiles(path);
        System.out.println("Duplicate files: ");
        if(duplicateFiles.isEmpty())
        {
            System.out.println("No duplicate files were found!");
        }
        else
        {
        for(String s:duplicateFiles)
        {
            System.out.println(s);
        }
        }
        System.out.println();
        System.out.println("Searching duplicates for : "+dirName+"  ...");
        ArrayList<String> duplicateDir = getDuplicateDir(dirName);
        if(duplicateDir.isEmpty())
        {
            System.out.println("No duplicate files were found!");
        }
        else
        {
        for(String s:duplicateDir)
        {
            System.out.println(s);
        }
        }
    }
}