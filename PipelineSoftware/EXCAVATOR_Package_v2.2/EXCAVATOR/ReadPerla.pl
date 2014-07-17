#!/usr/bin/perl

use warnings;
use Pod::Usage;
use Getopt::Long;
use strict;
use File::Path;
use Cwd 'abs_path';
	
######################################################################
#
#	Variables initialization with default values
#
######################################################################

our ($record,@words,@unlinkfiles,@SamplesVect);

our ($MAPQ)=0;						# Calling option "--mapq <integer>" sets the numeric variable $MAPQ=<integer>

our ($Program_Folder_Path,$Bam_File_Path,$Main_Output_Folder_Path,$Sample_Out_Folder_Path,$Sample_Name,$Sample_Label,$Assembly,$Target_Name,$Target_File_Path,$GC_Folder_Path,$RC_Folder_Path,$RCNorm_Folder_Path,$Images_Folder_Path,$R_Target_Folder,$R_Target_Path,$R_Norm_Path,$Input_File_Path,$mode,$Sample_Res_Folder_Path,$Sample_Plot_Folder_Path,$Sample_Data_Folder_Path);

our ($verbose,$help,$man);

######################################################################
#
#	Reading user's options
#
######################################################################

GetOptions('mapq=i'=>\$MAPQ,'mode=s'=>\$mode,'verbose|v'=>\$verbose,'help|h'=>\$help,'man|m'=>\$man,) or pod2usage ();
	
$help and pod2usage (-verbose=>1, -exitval=>1, -output=>\*STDOUT);
$man and pod2usage (-verbose=>2, -exitval=>1, -output=>\*STDOUT);
@ARGV or pod2usage (-verbose=>0, -exitval=>1, -output=>\*STDOUT);
@ARGV == 2 or pod2usage ("Syntax error");

######################################################################
#
#	Defining system variables
#
######################################################################

my ($myscriptname,$myscript,$workingfolder,$L1,$L2);

$myscript=abs_path($0);
$L1=length($myscript);
$L2=length("ReadPerla.pl");
$Program_Folder_Path=substr $myscript, 0, ($L1 - $L2 - 1);

print "Program folder is: $Program_Folder_Path\n";

$Input_File_Path=$ARGV[0];
$Main_Output_Folder_Path=$ARGV[1];

######################################################################
#
#	Checking system folders
#
######################################################################
	
print "Checking output folders...\n";
	 
if(-e $Main_Output_Folder_Path){ 
	print "'$Main_Output_Folder_Path' folder ready!\n";
}
else{
	mkpath($Main_Output_Folder_Path);
	if(-e $Main_Output_Folder_Path){
		print "'$Main_Output_Folder_Path' folder created!\n";
	}	
}

print "Checking output subfolders...\n";

if(-e "$Main_Output_Folder_Path/Data"){
	print "'$Main_Output_Folder_Path/Data' folder ready!\n";
}
else{
	mkdir ("$Main_Output_Folder_Path/Data");
	print "Creating '$Main_Output_Folder_Path/Data' folder...\n";
}

if(-e "$Main_Output_Folder_Path/Results"){
	print "'$Main_Output_Folder_Path/Results' folder ready!\n";
}
else{
	mkdir ("$Main_Output_Folder_Path/Results");
	print "Creating '$Main_Output_Folder_Path/Results' folder...\n";
}

if(-e "$Main_Output_Folder_Path/Plots"){
	print "'$Main_Output_Folder_Path/Plots' folder ready!\n";
}
else{
	mkdir ("$Main_Output_Folder_Path/Plots");
	print "Creating '$Main_Output_Folder_Path/Plots' folder...\n";
}
	
######################################################################
#
#	Removing empty lines from input file
#	and creating temporary files
#
######################################################################

my $range=99999;
my $ID=int(rand($range));
my $filename="input.$ID";

if(-e $filename){ 
	$ID=$ID+100000;
}

system qq(awk NF $Input_File_Path > input.$ID);

my $supportname="support.$ID";

if(-e $supportname){ 
	system qq(rm -f support.$ID);
}

open(SUPP,">>$Main_Output_Folder_Path/support.$ID") || die "Couldn't create support file.";

######################################################################
#
#	Reading input file
#
######################################################################

open(CHECKBOOK,"input.$ID") || die "Couldn't open the input file.";

while($record=<CHECKBOOK>){
	chomp($record);
	@words=split(' ',$record);
	
	$Target_Name=$words[0];
	$Assembly=$words[1];
	
	$Bam_File_Path=$words[2];
	
	$Sample_Name=$words[3];
	$Sample_Label=$words[4];
	
	$Sample_Out_Folder_Path="$Main_Output_Folder_Path/Data/$Sample_Name";
	$RC_Folder_Path="$Main_Output_Folder_Path/Data/$Sample_Name/RC";
	$RCNorm_Folder_Path="$Main_Output_Folder_Path/Data/$Sample_Name/RCNorm";
	$Images_Folder_Path="$Main_Output_Folder_Path/Data/$Sample_Name/Images";
	
	$GC_Folder_Path="$Program_Folder_Path/data/targets/$Assembly/$Target_Name/GCC";
	$R_Target_Folder="$Program_Folder_Path/data/targets/$Assembly/$Target_Name";
	$R_Target_Path="$R_Target_Folder/$Target_Name.RData";
	$R_Norm_Path="$Program_Folder_Path/lib/R/RCExomeNormalization.R";

	$Sample_Res_Folder_Path="$Main_Output_Folder_Path/Results/$Sample_Name";
	$Sample_Plot_Folder_Path="$Main_Output_Folder_Path/Plots/$Sample_Name";
	$Sample_Data_Folder_Path="$Main_Output_Folder_Path/Data/$Sample_Name";
	
	print "Working on sample $Sample_Name.\n";
	
	push(@SamplesVect,"$Sample_Name");
	
######################################################################
#
#	Checking samples' folders
#
######################################################################

	print "Checking sample '$Sample_Name' folders...\n";
	
	mkpath($RC_Folder_Path);
	mkpath($RCNorm_Folder_Path);
	mkpath($Images_Folder_Path);
	
	if($Sample_Label =~ m/T/){ 
		mkpath($Sample_Res_Folder_Path);
		mkpath($Sample_Plot_Folder_Path);
		mkpath($Sample_Data_Folder_Path);
	}
			
	print "...done!\n";

######################################################################
#
#	Read count
#
######################################################################
	
	print "Now working with SAMtools...\n";
	system qq($Program_Folder_Path/lib/bash/./FiltBam.sh $Bam_File_Path $MAPQ $Sample_Out_Folder_Path $Program_Folder_Path $Sample_Name $Target_Name $Assembly);
	print "Removing temporary files...\n";
	rmtree("$Sample_Out_Folder_Path/.tmp");
	print "...done!\n";

######################################################################
#
#	Writing support file for comparative filtering
#
######################################################################

	print SUPP "$Sample_Name,$Sample_Label\n";	

}

######################################################################
#
#	Data calculations
#
######################################################################

system qq(R --slave --args $Main_Output_Folder_Path,$R_Target_Folder,$Main_Output_Folder_Path/support.$ID,$mode,$Target_Name,$Program_Folder_Path < $Program_Folder_Path/lib/R/EXCAVATORNormalization.R);

system qq(R --slave --args $Main_Output_Folder_Path,$R_Target_Folder,$Main_Output_Folder_Path/support.$ID,$mode,$Target_Name,$Program_Folder_Path,$Assembly < $Program_Folder_Path/lib/R/EXCAVATORInference.R);

system qq(R --slave --args $Main_Output_Folder_Path,$Main_Output_Folder_Path/support.$ID < $Program_Folder_Path/lib/R/EXCAVATORPlots.R);

system qq(cp $Program_Folder_Path/ParameterFile.txt $Main_Output_Folder_Path/ParameterFile_$ID.txt);

print "Analyzed samples: \n";
print "@SamplesVect";

close(CHECKBOOK);
close(SUPP);
@unlinkfiles=("input.$ID");
unlink @unlinkfiles;

######################################################################
#
#	Documentation
#
######################################################################

=head1 SYNOPSIS 

 ReadPerla.pl [arguments] [options]

 Options:

       -h, --help                   Print help message.
       -m, --man                    Print complete documentation.
       -v, --verbose                Use verbose output.
           --mapq <integer>         Select mapping quality for .bam file filtering; if omitted default value is 0.
           --mode                   Select analysis mode. Available options: "pooling" or "somatic".

 Function:
 
 ReadPerla.pl performs read count calculations on .bam files with SAMtools and all calculations for "pooling" or "somatic" analyses. A list of .bam files that will be procesed must be written (in the proper format) in the input file.

 Example: 
 
 EXCAVATOR> perl ReadPerla.pl ReadInput.txt /Users/.../MyOutputFolder --mode pooling

=head1 OPTIONS

=over 8

=item B<--help>

Print a brief usage message and detailed explanation of options.

=item B<--man>

Print the complete manual of the script.

=item B<--verbose>

Use verbose output.

=item B<--mapq>

Sets the numeric value of the mapping quality for .bam file filtering; must be an integer number. If omitted default value is 0.

=back

=head1 DESCRIPTION

ReadPerla.pl is a Perl script which is part of the EXCAVATOR package. It includes all of the second step operations of the EXCAVATOR package. It calls SAMtools for read count calculations. To run ReadPerla.pl you MUST have already have at least one initialized target.

ReadPerla.pl requires two arguments and one command-line option (the "mode" option) to run properly. The first argument is the path to an input file. The default file name is ReadPerla.txt. It should be placed in EXCAVATOR main folder. The last argument is the path to the main output folder.

Each line in the input file refers to a .bam file that is gonna be processed. The following space-separated fields must be included in the line: target name, the assembly which was used for target initialization, .bam file path, name of the sample (which will be used as a prefix for output files), sample label.

The mapping quality value which is used by SAMtools can be set by means of the option --mapq when running ReadPerla.pl. If omitted default value is 0.

EXCAVATOR is freely available to the community for non-commercial use. For questions or comments, please contact "lorenzotattini@gmail.com".
=cut
