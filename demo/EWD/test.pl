$expr = $ARGV[0];

print "'$expr'\n";

$expr =~ /^(-?\d+)(\.\.(-?\d+))?/;
print "'$1' '$2' '$3'\n";
