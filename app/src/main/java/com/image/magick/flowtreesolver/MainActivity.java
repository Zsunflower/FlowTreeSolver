package com.image.magick.flowtreesolver;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.MobileAds;

public class MainActivity extends AppCompatActivity implements View.OnTouchListener
{
    public static final int ALL_PERMISSIONS_REQUEST = 100;
    public static int RESULT_LOAD_IMAGE = 1;
    public static String LOG_TAG = "Java_FlowTree";
    public static String solutionDir;
    public static int nSolved = 0;

    static
    {
        System.loadLibrary("Solver");
        validateKey("3c56f06b0cc20e5921100334985173eb");
    }

    private AdView mAdView;
    private ImageView btnGallery, btnLike, btnMore, btnFeedback;

    public static native void validateKey(String key);

    public static native void setSolutionDir(String dir);

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        MobileAds.initialize(this, "ca-app-pub-5118564015725949~8573632002");
        mAdView = findViewById(R.id.adView);
        AdRequest adRequest = new AdRequest.Builder().addTestDevice("36EEB4EE860265531C1386E371EF36D1").build();
        mAdView.loadAd(adRequest);
        mAdView.setAdListener(new AdListener()
        {
            @Override
            public void onAdLoaded()
            {
                // Code to be executed when an ad finishes loading.
            }

            @Override
            public void onAdFailedToLoad(int errorCode)
            {
                // Code to be executed when an ad request fails.
            }

            @Override
            public void onAdOpened()
            {
                // Code to be executed when an ad opens an overlay that
                // covers the screen.
            }

            @Override
            public void onAdLeftApplication()
            {
                // Code to be executed when the user has left the app.
            }

            @Override
            public void onAdClosed()
            {
                // Code to be executed when when the user is about to return
                // to the app after tapping on an ad.
            }
        });

        checkPermissions();
    }

    public void checkPermissions()
    {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED || ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED || ContextCompat.checkSelfPermission(this, Manifest.permission.INTERNET) != PackageManager.PERMISSION_GRANTED)
        {
            Log.d(LOG_TAG, "Permissions has not been granted yet");
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, ALL_PERMISSIONS_REQUEST);
        }
        else
        {
            init();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        switch (requestCode)
        {
            case (ALL_PERMISSIONS_REQUEST):
            {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED)
                {
                    Log.d(LOG_TAG, "Permissions was granted");
                    init();
                }
                else
                {
                    Log.d(LOG_TAG, "Permissions was not allowed");
                    checkPermissions();
                }
            }
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    private void init()
    {
        solutionDir = this.getFilesDir().getAbsolutePath();
        setSolutionDir(solutionDir);
        btnGallery = findViewById(R.id.start);
        btnLike = findViewById(R.id.like);
        btnMore = findViewById(R.id.more);
        btnFeedback = findViewById(R.id.feedback);

        btnGallery.setOnTouchListener(this);
        btnLike.setOnTouchListener(this);
        btnMore.setOnTouchListener(this);
        btnFeedback.setOnTouchListener(this);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == RESULT_LOAD_IMAGE && resultCode == RESULT_OK)
        {
            if (data != null)
            {
                Uri imagePathUri = data.getData();
                if (imagePathUri == null)
                    return;
                String[] filePathColumn = {MediaStore.Images.Media.DATA};
                Cursor cursor = getContentResolver().query(imagePathUri, filePathColumn, null, null, null);
                cursor.moveToFirst();
                int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
                String imagePath = cursor.getString(columnIndex);
                Log.d(LOG_TAG, imagePath);
                cursor.close();
                startSolutionActivity(imagePath);
            }
        }
    }

    private void startSolutionActivity(String path)
    {
        Intent i = new Intent(MainActivity.this, SolutionActivity.class);
        i.putExtra("path", path);
        startActivity(i);
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        switch (event.getAction())
        {
            case MotionEvent.ACTION_DOWN:
                ((ImageView) v).setColorFilter(Color.argb(255, 255, 0, 0));
                break;
            case MotionEvent.ACTION_UP:
                ((ImageView) v).setColorFilter(Color.argb(0, 0, 0, 0));
                switch (v.getId())
                {
                    case R.id.start:
                        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                        startActivityForResult(i, RESULT_LOAD_IMAGE);
                        break;
                    case R.id.like:
                        Uri uri = Uri.parse("market://details?id=" + getPackageName());
                        Intent goToMarket = new Intent(Intent.ACTION_VIEW, uri);
                        // To count with Play market backstack, After pressing back button,
                        // to taken back to our application, we need to add following flags to intent.
                        int flags = Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_MULTIPLE_TASK;
                        if (Build.VERSION.SDK_INT >= 21)
                        {
                            flags |= Intent.FLAG_ACTIVITY_NEW_DOCUMENT;
                        }
                        else
                        {
                            //noinspection deprecation
                            flags |= Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
                        }
                        goToMarket.addFlags(flags);
                        try
                        {
                            startActivity(goToMarket);
                        } catch (ActivityNotFoundException e)
                        {
                            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/apps/details?id=" + getPackageName())));
                        }
                        break;
                    case R.id.more:
                        Uri urix = Uri.parse("market://search?q=pub:MoonMoon");
                        Intent goToMarketx = new Intent(Intent.ACTION_VIEW, urix);
                        try
                        {
                            startActivity(goToMarketx);
                        } catch (ActivityNotFoundException e)
                        {
                            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/search?q=pub:MoonMoon")));
                        }
                        break;
                    case R.id.feedback:
                        Intent intent = new Intent(Intent.ACTION_SENDTO);
                        intent.setData(Uri.parse("mailto:")); // only email apps should handle this
                        intent.putExtra(Intent.EXTRA_EMAIL, new String[]{"leedrayazed94@gmail.com"});
                        intent.putExtra(Intent.EXTRA_SUBJECT, "Flow Solver feedback");
                        if (intent.resolveActivity(getPackageManager()) != null)
                        {
                            startActivity(intent);
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        return true;
    }
}
