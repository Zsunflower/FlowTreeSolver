package com.image.magick.flowtreesolver;

import android.os.Environment;
import android.util.Log;

import java.io.File;

public class Utils
{
    /* Checks if external storage is available for read and write */
    public static boolean isExternalStorageWritable()
    {
        String state = Environment.getExternalStorageState();
        return Environment.MEDIA_MOUNTED.equals(state);
    }

    public static String getPublicAlbumStorageDir(String albumName)
    {
        // Get the directory for the user's public pictures directory.
        File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), albumName);
        if (!file.mkdirs())
        {
            Log.e("FTSolver", "============ Public album storage directory already exist =============");
        }
        return file.getAbsolutePath();
    }

    public static String getExternalStorageDir(String directory)
    {
        File dir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/data/" + directory);
        if (!dir.mkdirs())
        {
            Log.d("FTSolver", "============ External storage directory already exist ===============");
        }
        return dir.getAbsolutePath();
    }

}
