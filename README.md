# Under Construction

This repo is currently under-construction.

OboeKit
=======

OboeKit is an Android library that simplifies the integration of low-latency audio features into your applications. Built upon the [Oboe](https://github.com/google/oboe) C++ library, OboeKit provides a high-level interface for:

-   Recording low-latency audio, ideal for creating karaoke applications.
-   Enabling low-latency ear-back (monitoring).
-   Applying audio effects in real-time during ear-back and on local audio recordings.
-   Applying audio effects to WebRTC tracks (currently exemplified with LiveKit).
-   Mixing music with audio at low latency (under 10ms) for WebRTC local tracks.
-   Playing multiple audio files simultaneously without drifting on Android devices.

Features
--------

-   **Low-Latency Audio Recording**: Capture audio with minimal delay, essential for applications like karaoke where timing is crucial.
-   **Low-Latency Ear-Back**: Provide performers with immediate feedback by routing their live audio input back to their headphones with negligible delay.
-   **Real-Time Audio Effects**: Apply effects such as reverb, echo, or equalization in real-time during ear-back sessions and on local recordings.
-   **WebRTC Audio Effects Integration**: Seamlessly apply audio effects to WebRTC tracks, with current examples demonstrating integration with LiveKit.
-   **Low-Latency Audio Mixing for WebRTC**: Mix music and live audio inputs with latency under 10ms for WebRTC local tracks, ensuring synchronized and high-quality audio streams.
-   **Simultaneous Audio Playback**: Play multiple audio files concurrently without drift, maintaining synchronization across all tracks.

Getting Started
---------------

### Prerequisites

-   Android Studio 4.0 or higher
-   Android NDK r17 or higher
-   Minimum API Level 16 (Android 4.1, Jelly Bean)

[//]: # ()
[//]: # (### Installation)

[//]: # ()
[//]: # (1.  **Clone the Repository**:)

[//]: # ()
[//]: # (    bash)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `git clone https://github.com/yourusername/OboeKit.git`)

[//]: # ()
[//]: # (2.  **Add OboeKit to Your Project**:)

[//]: # ()
[//]: # (    -   **Using Android Studio**:)

[//]: # ()
[//]: # (        -   Open your project in Android Studio.)

[//]: # (        -   Navigate to `File` > `New` > `Import Module`.)

[//]: # (        -   Select the `OboeKit` directory from the cloned repository.)

[//]: # (        -   Follow the prompts to add the module to your project.)

[//]: # (    -   **Manual Integration**:)

[//]: # ()
[//]: # (        -   Copy the `OboeKit` directory into your project's root directory.)

[//]: # ()
[//]: # (        -   In your project's `settings.gradle` file, include the module:)

[//]: # ()
[//]: # (            groovy)

[//]: # ()
[//]: # (            CopyEdit)

[//]: # ()
[//]: # (            `include ':app', ':OboeKit'`)

[//]: # ()
[//]: # (        -   In your app module's `build.gradle` file, add a dependency on OboeKit:)

[//]: # ()
[//]: # (            groovy)

[//]: # ()
[//]: # (            CopyEdit)

[//]: # ()
[//]: # (            `dependencies {)

[//]: # (            implementation project&#40;':OboeKit'&#41;)

[//]: # (            }`)

[//]: # ()
[//]: # (### Usage)

[//]: # ()
[//]: # (1.  **Initialize OboeKit**:)

[//]: # ()
[//]: # (    Before using OboeKit's features, initialize it in your application's `onCreate` method:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `@Override)

[//]: # (    public void onCreate&#40;&#41; {)

[//]: # (    super.onCreate&#40;&#41;;)

[//]: # (    OboeKit.initialize&#40;this&#41;;)

[//]: # (    }`)

[//]: # ()
[//]: # (2.  **Recording Low-Latency Audio**:)

[//]: # ()
[//]: # (    To start recording audio with low latency:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.startRecording&#40;new OboeKit.RecordingCallback&#40;&#41; {)

[//]: # (    @Override)

[//]: # (    public void onAudioData&#40;byte[] data&#41; {)

[//]: # (    // Handle the recorded audio data)

[//]: # (    })

[//]: # (    }&#41;;`)

[//]: # ()
[//]: # (3.  **Enabling Low-Latency Ear-Back**:)

[//]: # ()
[//]: # (    To enable ear-back for live monitoring:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.enableEarBack&#40;true&#41;;`)

[//]: # ()
[//]: # (4.  **Applying Real-Time Audio Effects**:)

[//]: # ()
[//]: # (    Apply effects during ear-back or recording:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.applyEffect&#40;OboeKit.Effect.REVERB, true&#41;;`)

[//]: # ()
[//]: # (5.  **Integrating with WebRTC &#40;LiveKit Example&#41;**:)

[//]: # ()
[//]: # (    To apply audio effects to WebRTC tracks:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.attachToWebRTCTrack&#40;liveKitTrack, new OboeKit.Effect[]{OboeKit.Effect.ECHO}&#41;;`)

[//]: # ()
[//]: # (6.  **Mixing Music with Live Audio for WebRTC**:)

[//]: # ()
[//]: # (    To mix background music with live audio input:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.mixWithMusic&#40;backgroundMusicFile, liveAudioInput, new OboeKit.MixCallback&#40;&#41; {)

[//]: # (    @Override)

[//]: # (    public void onMixedAudio&#40;byte[] mixedData&#41; {)

[//]: # (    // Handle the mixed audio data)

[//]: # (    })

[//]: # (    }&#41;;`)

[//]: # ()
[//]: # (7.  **Playing Multiple Audio Files Simultaneously**:)

[//]: # ()
[//]: # (    To play multiple audio files without drift:)

[//]: # ()
[//]: # (    java)

[//]: # ()
[//]: # (    CopyEdit)

[//]: # ()
[//]: # (    `OboeKit.playAudioFiles&#40;new String[]{file1, file2, file3}&#41;;`)

[//]: # ()
[//]: # (Documentation)

[//]: # (-------------)

[//]: # ()
[//]: # (For detailed documentation and advanced usage, please refer to the [OboeKit Wiki]&#40;https://github.com/yourusername/OboeKit/wiki&#41;.)

[//]: # ()
[//]: # (Contributing)

[//]: # (------------)

[//]: # ()
[//]: # (We welcome contributions! Please see our [Contributing Guidelines]&#40;https://github.com/yourusername/OboeKit/blob/main/CONTRIBUTING.md&#41; for more information.)