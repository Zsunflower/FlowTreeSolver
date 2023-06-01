package com.image.magick.flowtreesolver;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import com.github.chrisbanes.photoview.PhotoView;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.InterstitialAd;

import java.io.File;
import java.io.FileOutputStream;

public class SolutionActivity extends AppCompatActivity implements View.OnClickListener
{
    ImageView btnShare, btnSave, btnRate;
    private String path;
    private PhotoView solutionView;
    private Bitmap bitmap;
    private InterstitialAd mInterstitialAd;

    public native boolean solve(String path);

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_solution);
        Intent i = getIntent();
        path = i.getStringExtra("path");

        btnShare = findViewById(R.id.share);
        btnSave = findViewById(R.id.save);
        btnRate = findViewById(R.id.rate);
        btnShare.setOnClickListener(this);
        btnSave.setOnClickListener(this);
        btnRate.setOnClickListener(this);

        solutionView = findViewById(R.id.solution);
        loadImage();
        loadAds();
        new Thread(new Runnable()
        {
            public void run()
            {
                solve(path);
            }
        }).start();
    }

    private void loadImage()
    {
        File imgFile = new File(path);
        if (imgFile.exists())
        {
            bitmap = BitmapFactory.decodeFile(imgFile.getAbsolutePath());
            solutionView.setImageBitmap(bitmap);
        }
    }

    private void loadAds()
    {
        mInterstitialAd = new InterstitialAd(this);
        mInterstitialAd.setAdUnitId("ca-app-pub-5118564015725949/4712130816");
        mInterstitialAd.loadAd(new AdRequest.Builder().build());
        mInterstitialAd.setAdListener(new AdListener()
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
                // Code to be executed when the ad is displayed.
            }

            @Override
            public void onAdLeftApplication()
            {
                // Code to be executed when the user has left the app.
            }

            @Override
            public void onAdClosed()
            {
                // Code to be executed when when the interstitial ad is closed.
                SolutionActivity.super.onBackPressed();
            }
        });
    }

    private void onJNISuccess()
    {
        File imgFile = new File(MainActivity.solutionDir + "/" + "solution.png");
        if (imgFile.exists())
        {
            Log.d("FTSolver", "Got a solution!");
            ++MainActivity.nSolved;
            bitmap = BitmapFactory.decodeFile(imgFile.getAbsolutePath());
            Runnable runnable = new Runnable()
            {
                @Override
                public void run()
                {
                    solutionView.setImageBitmap(bitmap);
                }
            };
            runOnUiThread(runnable);
        }
    }

    private void onJNIFailed()
    {
        Log.d("FTSolver", "Game has no solution");
        Runnable runnable = new Runnable()
        {
            @Override
            public void run()
            {
                Toast.makeText(getApplicationContext(), "Game has no solution!", Toast.LENGTH_LONG).show();
            }
        };
        runOnUiThread(runnable);
    }

    @Override
    public void onClick(View view)
    {
        switch (view.getId())
        {
            case R.id.rate:
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
            case R.id.save:
                if (Utils.isExternalStorageWritable())
                {
                    saveImageTo(bitmap, Utils.getPublicAlbumStorageDir("FTSolutions"), "FTSolution_" + System.currentTimeMillis() + ".png");
                }
                else
                {
                    Log.d("FTSolver", "============ External storage is not available ===========");
                    saveImageTo(bitmap, Utils.getExternalStorageDir(getPackageName()), "FTSolution_" + System.currentTimeMillis() + ".png");
                }
                break;
            case R.id.share:
                try
                {
                    File file = new File(this.getExternalCacheDir(), "FTSolution.png");
                    FileOutputStream fOut = new FileOutputStream(file);
                    bitmap.compress(Bitmap.CompressFormat.PNG, 100, fOut);
                    fOut.flush();
                    fOut.close();
                    final Intent intent = new Intent(android.content.Intent.ACTION_SEND);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    Uri photoURI = FileProvider.getUriForFile(this, getPackageName() + ".fileprovider", file);
                    intent.putExtra(Intent.EXTRA_STREAM, photoURI);
                    intent.setType("image/*");
                    startActivity(Intent.createChooser(intent, "Share image via"));
                } catch (Exception e)
                {
                    Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
                    e.printStackTrace();
                }

                break;
            default:
                break;
        }
    }

    public void saveImageTo(Bitmap bitmap, String directory, String fileName)
    {
        File file = new File(directory, fileName);
        FileOutputStream fos;
        try
        {
            fos = new FileOutputStream(file);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);
            fos.flush();
            fos.close();
            MediaStore.Images.Media.insertImage(this.getContentResolver(), file.getAbsolutePath(), file.getName(), "FTSolutions");
            Toast.makeText(this, "Saved to " + file.getAbsolutePath(), Toast.LENGTH_LONG).show();
            Log.d("FTSolver", "============ Solution saved to " + file.getAbsolutePath() + " ============");
        } catch (Exception e)
        {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    @Override
    public void onBackPressed()
    {
        if (mInterstitialAd.isLoaded() && MainActivity.nSolved % 5 == 0)
        {
            Log.d("FTSolver", "Show admod video");
            mInterstitialAd.show();
        }
        else
        {
            super.onBackPressed();
        }
    }
}
