*Copied from on 8 Jan 2014, Specification:*
<http://www.music-ir.org/mirex/wiki/2013:Audio_Music_Similarity_and_Retrieval#Distance_matrix_output_files>

=Distance matrix output files=

Participants should return one of two available output file formats, a full
distance matrix or a sparse distance matrix. The sparse distance matrix format
is preferred (as the dense distance matrices can be very large).


## Sparse Distance Matrix ##

If computation or exhaustive search is a concern or not a normal output of the
indexing algorithm employed, the sparse distance matric format detailed below
may be used:

A simple ASCII file listing a name for the algorithm and the top 100 search
results for every track in the collection.

This file should start with a header line with a name for the algorithm and
should be followed by the results for one query per line, prefixed by the
filename portion of the query path. This should be followed by a tab character
and a tab separated, ordered list of the top 100 search results. Each result
should include the result filename (e.g. a034728.wav) and the distance (e.g.
17.1 or 0.23) separated by a a comma.

```
MyAlgorithm (my.email@address.com)
<example 1 filename>\t<result 1 name>,<result 1 distance>,\t<result 2 name>,<result 2 distance>, ... \t<result 100 name>,<result 100 distance>
<example 2 filename>\t<result 1 name>,<result 1 distance>,\t<result 2 name>,<result 2 distance>, ... \t<result 100 name>,<result 100 distance>
...
```

which might look like:
```
MyAlgorithm (my.email@address.com)
a009342.wav	b229311.wav,0.16	a023821.wav,0.19	a001329,0.24  ... etc.
a009343.wav	a661931.wav,0.12	a043322.wav,0.17	c002346,0.21  ... etc.
a009347.wav	a671239.wav,0.13	c112393.wav,0.20	b083293,0.25  ... etc.
...
```

The path to which this list file should be written must be accepted as a parameter on the command line.

## Full Distance Matrix ##

Full distance matrix files should be generated in the the following format:

A simple ASCII file listing a name for the algorithm on the first line,
Numbered paths for each file appearing in the matrix, these can be in any
order (i.e. the files dont have to be i the same order as they appeared in the
list file) but should index into the columns/rows of of the distance matrix.
A line beginning with Q/R followed by a tab and tab separated list of the
numbers 1 to N, where N is the files covered by the matrix. One line per file
in the matrix give the distances of that files to each other file in the
matrix. All distances should be zero or positive (0.0+) and should not be
infinite or NaN. Values should be separated by a single tab character.
Obviously the diagonal of the matrix (distance or a track to itself) should be
zero. 

```
Distance matrix header text with system name
1\t</path/to/audio/file/1.wav>
2\t</path/to/audio/file/2.wav>
3\t</path/to/audio/file/3.wav>
...
N\t</path/to/audio/file/N.wav>
Q/R\t1\t2\t3\t...\tN
1\t0.0\t<dist 1 to 2>\t<dist 1 to 3>\t...\t<dist 1 to N>
2\t<dist 2 to 1>\t0.0\t<dist 2 to 3>\t...\t<dist 2 to N>
3\t<dist 3 to 2>\t<dist 3 to 2>\t0.0\t...\t<dist 3 to N>
...\t...\t...\t...\t...\t...
N\t<dist N to 1>\t<dist N to 2>\t<dist N to 3>\t...\t0.0
```
which might look like:

```
Example distance matrix 0.1
1    /path/to/audio/file/1.wav
2    /path/to/audio/file/2.wav
3    /path/to/audio/file/3.wav
4    /path/to/audio/file/4.wav
Q/R   1        2        3        4
1     0.00000  1.24100  0.2e-4   0.42559
2     1.24100  0.00000  0.62640  0.23564
3     50.2e-4  0.62640  0.00000  0.38000
4     0.42559  0.23567  0.38000  0.00000
```


