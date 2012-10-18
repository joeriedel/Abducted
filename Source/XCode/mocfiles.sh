# Runs QtMoc on crow source

ROOT=$GLOBAL_PROJECT_ROOT/Source

function m() {
	echo $QT_MOC -o$BASE/"moc_"$1".cc" $BASE/$1".h"
	$QT_MOC -o$BASE/"moc_"$1".cc" $BASE/$1".h"
}

BASE=$ROOT

m "AbductedLauncher"
