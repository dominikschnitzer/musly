Musly
=====

(c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

Musly is a program and library for high performance audio music similarity
computation. Musly only uses the audio signal when computing similarities!
No meta data is used in the similarity computation process.

The source code is released under the MPL 2.0 see the file <musly/COPYING>

* The main Musly website is located at: <http://www.musly.org>
* Please fork me at: <https://github.com/dominikschnitzer/musly>
* Please report any Musly bug at:
  <https://github.com/dominikschnitzer/musly/issues>
* Documentation is found in the local <musly/doc> directory and on the main
  website.


## Version History ##

### VERSION 0.1 ###
Released on 30 Jan 2014.

The first public Musly release. It includes two basic audio music similarity
measures:

-   *mandelellis* (as published in "M. Mandel and D. Ellis: Song-level
    features and support vector machines for music classification. In the
    proceedings of the 6th International Conference on Music Information Retrieval,
    ISMIR, 2005)
    
-   *timbre* the mandelellis similarity measure tweaked for best
    results. We use a 25 MFCCs representation, the Jensen-Shannon divergence
    and normalize the similarities with Mutual Proximity
    (D. Schnitzer et al.: Using mutual proximity to improve
    content-based audio similarity. In the proceedings of the 12th
    International Society for Music Information Retrieval
    Conference, ISMIR, 2011).

For benchmarks and qualitative comparisons between other audio music
similarity measures visit <http://www.musly.org>.


## Command Line Tool ##

The command line interface is able to:

* Generate M3U playlists for music collections.
* Output full similarity matrices for more in-depth research of the
  audio music similarity functions. It uses the music-ir.org MIREX format
  (see <musly/doc/MIREX-DistanceMatrix.md>)
* Additionally the music similarity features can be ouput in text format
  to ease reuse of the features.
  
The command line tool is called "musly". Use "musly -h" to read about all
available options. See <http://www.musly.org> for more information.


## Library ##

Please see the library documentation in <doc/html> directory or on-line.


## Development ##

To help Musly development and fix bugs the most convenient way is to use
Eclipse. To generate an Eclipse project use:

```
cmake -G"Eclipse CDT4 - Unix Makefiles"\
      -DCMAKE_ECLIPSE_EXECUTABLE=/path/to/eclipse/eclipse \
      /path/to/musly-src
```

from an empty directory. The directory is filled with Eclipse project
information which can be imported as an Eclipse project.


