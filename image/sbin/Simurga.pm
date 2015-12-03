package Simurga;

require Exporter;
@EXPORT_OK = qw(connect_db commit prompt prompt_long prompt_bool print_current_long);

use strict;
use DBI;
use DBD::mysql;

my $DB;
my $DATA;

sub connect_db
{
	my $data_dir = $ENV{'SIMURGA_DATA'} || $ENV{'SIMURGA_IMAGE'} || '/usr/local';
	my $dbconf = "$data_dir/etc/db.conf";
	my $dbmanage_conf = "$data_dir/etc/db-manage.conf";
	$dbconf = $dbmanage_conf if -r $dbmanage_conf;
	-r $dbconf || die "Could not find database config file";

	# read database config
	my $config = {};
	my $fd;
	open($fd, $dbconf) || die $!;
	while (my $line = <$fd>)
	{
		chomp($line);
		if ($line =~ /^([\w\d_]+)\s*=\s*([^\s]+)$/)
		{
			$config->{$1} = $2;
		}
	}
	close($fd);

	my $dsn = "DBI:mysql:database=" . $config->{'db'};
	$dsn .= ";host=" . $config->{'host'} if $config->{'host'};
	$dsn .= ";port=" . $config->{'port'} if $config->{'port'};
	$dsn .= ";mysql_socket=" . $config->{'unix_socket'} if $config->{'unix_socket'};

	my $dbh = DBI->connect($dsn, $config->{'user'}, $config->{'passwd'}, {RaiseError => 1, mysql_enable_utf8 => 1, AutoCommit => 0});
	die $! unless $dbh;

	$DB = $dbh;
	$DATA = $data_dir;
	return $dbh;
}

sub commit
{
	$DB->commit;
}


sub quote
{
	my $table = shift;

	my @result = ();
	foreach my $key (@_)
	{
		push(@result, $DB->quote($table->{$key}));
	}

	return @result;
}


sub prompt
{
	my $title = shift;
	print "$title > ";
	my $result = <STDIN>;
	chomp($result);
	return $result;
}

sub prompt_long
{
	my $title = shift;
	print "Enter $title ('.' on blank line to end):\n\n";

	my $result = '';
	while (my $line = <STDIN>)
	{
		chomp($line);
		last if $line eq '.';
		$result .= "\n" if $result;
		$result .= $line;
	}

	print "\n";

	return $result;
}

sub prompt_bool
{
	my $title = shift;
	while (1)
	{
		print "$title [y/n] > ";
		my $result = <STDIN>;
		chomp $result;
		return 1 if $result eq 'y' or $result eq 'Y';
		return 0 if $result eq 'n' or $result eq 'N';
	}
}

sub print_current_long
{
	my $title = shift;
	my $value = shift;
	print "Current $title :\n\n$value\n\n";
}


sub _get_save
{
	my $object = shift;
	my $key = shift;
	my $value = shift;
	$object->{$key} = $value if $value;
	return $object->{$key};
}


package Simurga::Color;


sub _check_name
{
	my $name = shift;
	return $name =~ /^[\w\d_\-]+$/;
}

sub _check_code
{
	my $code = shift;
	return $code =~ /^[\da-fA-F]{6}$/;
}

sub new
{
	my $class = shift;
	my $data = shift;

	$data->{'desc_ru'} ||= '';
	$data->{'desc_en'} ||= '';
	die "Need name and code" unless $data->{'name'} && $data->{'code'};
	die "Wrong name" unless _check_name($data->{'name'});
	die "Wrong code" unless _check_code($data->{'code'});

	return bless $data, $class;
}

sub create
{
	my $class = shift;
	my $self = $class->new(shift);
	$self->save();
	return $self;
}

sub by_id
{
	my $class = shift;
	my $id = shift;

	my $sth = $DB->prepare('select * from `color` where `id` = ? limit 1');
	$sth->execute($id);
	die "Invalid id" unless $sth->rows;
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub by_name
{
	my $class = shift;
	my $name = shift;

	my $sth = $DB->prepare('select * from `color` where `name` = ? limit 1');
	$sth->execute($name);
	die "Invalid color name '$name'" unless $sth->rows;
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub save
{
	my $self = shift;
	die "Need DB connection" unless $DB;

	if ($self->{'id'})
	{
		$DB->do(
			"update `color` set `name` = ?, `code` = ?, `desc_ru` = ?, `desc_en` = ? " .
			"where `id` = ?", undef,
			$self->{'name'}, $self->{'code'}, $self->{'desc_ru'}, $self->{'desc_en'},
			$self->{'id'});
	}
	else
	{
		$DB->do(
			"insert into `color` (`name`, `code`, `desc_ru`, `desc_en`) values (?, ?, ?, ?)", undef,
			$self->{'name'}, $self->{'code'}, $self->{'desc_ru'}, $self->{'desc_en'});
		$self->{'id'} = $DB->{'mysql_insertid'};
	}
}

sub all
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my @result = ();

	my $sth = $DB->prepare("select * from `color` order by `name`");
	$sth->execute();
	while (my $ref = $sth->fetchrow_hashref)
	{
		push(@result, Simurga::Color->new($ref));
	}
	$sth->finish();

	return @result;
}

sub prompt
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my @all = $class->all;
	my $map = {};

	print "Available colors:\n";
	foreach my $color (@all)
	{
		$map->{$color->name} = $color;
		print "  [" . $color->name . "] " . $color->desc_ru . "\n";
	}
	while (1)
	{
		print "Choose one > ";
		my $name = <STDIN>;
		chomp($name);
		return $map->{$name} if $map->{$name};
	}
}

sub code
{
	my $self = shift;
	my $value = shift;
	if ($value)
	{
		_check_code($value) || die "Wrong code";
		$self->{'code'} = $value;
	}
	return $self->{'code'};
}

sub id { return shift->{'id'}; }
sub name { return shift->{'name'}; }
sub desc_ru { return Simurga::_get_save(shift, 'desc_ru', shift); }
sub desc_en { return Simurga::_get_save(shift, 'desc_en', shift); }


package Simurga::Material;


sub new
{
	my $class = shift;
	my $data = shift;

	$data->{'desc_ru'} ||= '';
	$data->{'desc_en'} ||= '';
	die "Need name" unless $data->{'name'};

	return bless $data, $class;
}

sub create
{
	my $class = shift;
	my $self = $class->new(shift);
	$self->save();
	return $self;
}

sub save
{
	my $self = shift;
	die "Need DB connection" unless $DB;

	if ($self->{'id'})
	{
		$DB->do(
			"update `material` set `name` = ?, `desc_ru` = ?, `desc_en` = ? " .
			"where `id` = ?", undef,
			$self->{'name'}, $self->{'desc_ru'}, $self->{'desc_en'},
			$self->{'id'});
	}
	else
	{
		$DB->do(
			"insert into `material` (`name`, `desc_ru`, `desc_en`) values (?, ?, ?)", undef,
			$self->{'name'}, $self->{'desc_ru'}, $self->{'desc_en'});
		$self->{'id'} = $DB->{'mysql_insertid'};
	}
}

sub by_id
{
	my $class = shift;
	my $id = shift;

	my $sth = $DB->prepare('select * from `material` where `id` = ? limit 1');
	$sth->execute($id);
	die "Invalid id" unless $sth->rows;
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub all
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my @result = ();

	my $sth = $DB->prepare("select * from `material` order by `name`");
	$sth->execute();
	while (my $ref = $sth->fetchrow_hashref)
	{
		push(@result, Simurga::Material->new($ref));
	}
	$sth->finish();

	return @result;
}

sub prompt
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my @all = $class->all;
	my $map = {};

	print "Available materials:\n";
	foreach my $material (@all)
	{
		$map->{$material->id} = $material;
		print "  [" . $material->id . "] " . $material->name . "\n";
	}
	while (1)
	{
		print "Choose one > ";
		my $id = <STDIN>;
		$id += 0;
		return $map->{$id} if $map->{$id};
	}
}

sub id { return shift->{'id'}; }
sub name { return Simurga::_get_save(shift, 'name', shift); }
sub desc_ru { return Simurga::_get_save(shift, 'desc_ru', shift); }
sub desc_en { return Simurga::_get_save(shift, 'desc_en', shift); }


package Simurga::Model;


sub new
{
	my $class = shift;
	my $data = shift;

	$data->{'short_title_ru'} ||= '';
	$data->{'short_title_en'} ||= '';
	$data->{'title_en'} = $data->{'title_ru'} unless $data->{'title_en'};
	$data->{'title_ru'} = $data->{'title_en'} unless $data->{'title_ru'};
	die "Need title" unless $data->{'title_ru'};

	return bless $data, $class;
}

sub create
{
	my $class = shift;
	my $self = $class->new(shift);
	$self->save();
	return $self;
}

sub by_id
{
	my $class = shift;
	my $id = shift;

	my $sth = $DB->prepare('select * from `model` where `id` = ? limit 1');
	$sth->execute($id);
	die "Invalid id" unless $sth->rows;
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub _rename_all
{
	my ($old_id, $new_id) = @_;

	my $root = "$DATA/pub/model";
	my $dh;
	opendir($dh, $root);
	my @items = readdir($dh);
	closedir($dh);

	foreach my $item(@items)
	{
		next if $item =~ /^\./;
		next unless -d "$root/$item";
		next unless index($item, $old_id . '-') == 0;
		my $new_name = $item;
		substr($new_name, 0, length($old_id)) = $new_id;
		rename("$root/$item", "$root/$new_name") || die $!;
	}
}

sub save
{
	my $self = shift;
	die "Need DB connection" unless $DB;

	my $sth = $DB->prepare('select `id` from `model` where `id` = ? limit 1');
	$sth->execute($self->{'id'});
	my $found = $sth->rows;
	$sth->finish();

	if ($found)
	{
		$DB->do(
			"update `model` set `short_title_ru` = ?, `short_title_en` = ?, `title_ru` = ?, `title_en` = ? " .
			"where `id` = ?", undef,
			$self->{'short_title_ru'}, $self->{'short_title_en'}, $self->{'title_ru'}, $self->{'title_en'},
			$self->{'id'});
	}
	else
	{
		$DB->do(
			"insert into `model` (`id`, `short_title_ru`, `short_title_en`, `title_ru`, `title_en`) values (?, ?, ?, ?, ?)", undef,
			$self->{'id'}, $self->{'short_title_ru'}, $self->{'short_title_en'}, $self->{'title_ru'}, $self->{'title_en'});
	}

	if ($self->{'_old_id'})
	{
		_rename_all($self->{'_old_id'} + 0, $self->{'id'} + 0);
		delete $self->{'_old_id'};
	}
}

sub all
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my @result = ();

	my $sth = $DB->prepare("select * from `model` order by `id`");
	$sth->execute();
	while (my $ref = $sth->fetchrow_hashref)
	{
		push(@result, Simurga::Model->new($ref));
	}
	$sth->finish();

	return @result;
}

sub short_title_ru { return Simurga::_get_save(shift, 'short_title_ru', shift); }
sub short_title_en { return Simurga::_get_save(shift, 'short_title_en', shift); }
sub title_ru { return Simurga::_get_save(shift, 'title_ru', shift); }
sub title_en { return Simurga::_get_save(shift, 'title_en', shift); }

sub id
{
	my $self = shift;
	my $new_id = shift;
	if ($new_id += 0)
	{
		$self->{'_old_id'} ||= $self->{'id'} if $self->{'id'};
		$self->{'id'} = $new_id;
	}
	return $self->{'id'};
}


package Simurga::ModelColor;


my $SIZE_XS   = 1 << 3;
my $SIZE_S    = 1 << 4;
my $SIZE_M    = 1 << 5;
my $SIZE_L    = 1 << 6;
my $SIZE_XL   = 1 << 7;
my $SIZE_XXL  = 1 << 8;
my $SIZE_XXXL = 1 << 9;


sub size_to_string
{
	my $num = shift;
	my @sizes = ('XS','S','M','L','XL','XXL','XXXL');
	my @result = ();
	for (my $i = 0; $i < @sizes; $i++)
	{
		my $mask = 1 << (3 + $i);
		push(@result, @sizes[$i]) if ($num & $mask) != 0;
	}
	return join(',', @result);
}

sub string_to_size
{
	my $str = shift;
	my @sizes = split(',', $str);
	my $table = {
		'XS' => $SIZE_XS,
		'S' => $SIZE_S,
		'M' => $SIZE_M,
		'L' => $SIZE_L,
		'XL' => $SIZE_XL,
		'XXL' => $SIZE_XXL,
		'XXXL' => $SIZE_XXXL
	};

	my $result = 0;
	foreach my $item(@sizes)
	{
		$result |= $table->{$item} if $table->{$item};
	}

	return $result;
}


sub new
{
	my $class = shift;
	my $data = shift;

	die "Need model_id" unless $data->{'model_id'};
	die "Need color_id" unless $data->{'color_id'};
	$data->{'num_images'} = 0 unless $data->{'num_images'};
	$data->{'sizes'} = string_to_size($data->{'sizes'}) if $data->{'sizes'} =~ /[SMLX]/;
	$data->{'price_ru'} = 0 unless $data->{'price_ru'};
	$data->{'price_en'} = 0 unless $data->{'price_en'};
	$data->{'desc_en'} = $data->{'desc_ru'} unless $data->{'desc_en'};
	$data->{'desc_ru'} = $data->{'desc_en'} unless $data->{'desc_ru'};

	return bless $data, $class;
}

sub create
{
	my $class = shift;
	my $self = $class->new(shift);
	$self->save();
	return $self;
}

sub by_id
{
	my $class = shift;
	my $id = shift;

	my $sth = $DB->prepare('select * from `model_color` where `id` = ? limit 1');
	$sth->execute($id);
	die "Invalid id" unless $sth->rows;
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub by_model_and_color
{
	my $class = shift;
	my $model_id = shift(@_) + 0;
	my $color_name = shift;

	my $color = Simurga::Color->by_name($color_name);
	my $sth = $DB->prepare('select * from `model_color` where `model_id` = ? and `color_id` = ? limit 1');
	$sth->execute($model_id, $color->id);
	my $result = $class->new($sth->fetchrow_hashref);
	$sth->finish();
	return $result;
}

sub next_id
{
	my $sth = $DB->prepare('select max(`id`) from `model_color`');
	$sth->execute();
	my $result = $sth->rows ? $sth->fetchrow_arrayref->[0] + 1 : 1;
	$sth->finish();
	return $result;
}

sub save
{
	my $self = shift;

	if ($self->{'id'})
	{
		$DB->do(
			"update `model_color` set " .
			"`model_id` = ?, `color_id` = ?, `num_images` = ?, `sizes` = ?, `material_id` = ?, " .
			"`price_ru` = ?, `price_en` = ?, `special_price_ru` = ?, `special_price_en` = ?, `desc_ru` = ?, `desc_en` = ? " .
			"where `id` = ?", undef,
			$self->{'model_id'}, $self->{'color_id'}, $self->{'num_images'}, $self->{'sizes'}, $self->{'material_id'},
			$self->{'price_ru'}, $self->{'price_en'},
			$self->{'special_price_ru'}, $self->{'special_price_en'},
			$self->{'desc_ru'}, $self->{'desc_en'},
			$self->{'id'});
	}
	else
	{
		$self->{'id'} = next_id;
		$DB->do(
			"insert into `model_color` (`id`, `model_id`, `color_id`, `num_images`, `sizes`, `material_id`, " .
			"`price_ru`, `price_en`, `special_price_ru`, `special_price_en, `desc_ru`, `desc_en`) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", undef,
			$self->{'id'}, $self->{'model_id'}, $self->{'color_id'}, $self->{'num_images'}, $self->{'sizes'}, $self->{'material_id'},
			$self->{'price_ru'}, $self->{'price_en'},
			$self->{'special_price_ru'}, $self->{'special_price_en'},
			$self->{'desc_ru'}, $self->{'desc_en'});
	}
}

sub move_to
{
	my $self = shift;
	my $new_id = shift(@_) + 0;
	my $old_id = $self->id + 0;
	my $tmp_id = next_id + 0;

	return if $new_id == $old_id;

	my $move = $DB->prepare("update `model_color` set `id` = ? where `id` = ?");

	# 1. Move current item to the temporary position.
	$move->execute($old_id, $tmp_id);

	# 2. Move another items to make new_id free
	if ($old_id < $new_id)
	{
		for (my $id = $old_id; $id < $new_id; $id++)
		{
			$move->execute($id, $id+1);
		}
	}
	else
	{
		for (my $id = $old_id; $id > $new_id; $id--)
		{
			$move->execute($id, $id-1);
		}
	}

	# 3. Move temporary item to the new id
	$move->execute($tmp_id, $new_id);

	$move->finish();

	$self->{'id'} = $new_id;
}

sub all
{
	my $class = shift;
	die "Need DB connection" unless $DB;

	my $start = shift;
	my $limit = shift;

	my @result = ();

	my $query = "select * from `model_color` order by `id` desc";
	$query .= " limit $limit" if $limit += 0;
	$query .= " offset $start" if $start += 0;

	my $sth = $DB->prepare($query);
	$sth->execute();
	while (my $ref = $sth->fetchrow_hashref)
	{
		push(@result, Simurga::ModelColor->new($ref));
	}
	$sth->finish();

	return @result;
}

sub id { return shift->{'id'}; }
sub model_id { return shift->{'model_id'}; }
sub color_id { return shift->{'color_id'}; }
sub material_id { return shift->{'material_id'}; }
sub num_images { return shift->{'num_images'}; }
sub sizes { return size_to_string(shift->{'sizes'}); }
sub price_ru { return Simurga::_get_save(shift, 'price_ru', shift); }
sub price_en { return Simurga::_get_save(shift, 'price_en', shift); }
sub special_price_ru { return Simurga::_get_save(shift, 'special_price_ru', shift); }
sub special_price_en { return Simurga::_get_save(shift, 'special_price_en', shift); }
sub remove_special_price_ru { delete shift->{'special_price_ru'}; }
sub remove_special_price_en { delete shift->{'special_price_en'}; }
sub desc_ru { return Simurga::_get_save(shift, 'desc_ru', shift); }
sub desc_en { return Simurga::_get_save(shift, 'desc_en', shift); }

sub model
{
	my $self = shift;
	my $new_model = shift;
	if ($new_model)
	{
		$self->{'model_id'} = $new_model->id;
		return $new_model;
	}
	return Simurga::Model->by_id($self->model_id);
}

sub color
{
	my $self = shift;
	my $new_color = shift;
	if ($new_color)
	{
		$self->{'color_id'} = $new_color->id;
		return $new_color;
	}
	return Simurga::Color->by_id($self->color_id);
}

sub material
{
	my $self = shift;
	my $new_material = shift;
	if ($new_material)
	{
		$self->{'material_id'} = $new_material->id;
		return $new_material;
	}
	return Simurga::Material->by_id($self->material_id);
}


sub _big
{
	my ($model_id, $color_name, $num) = @_;
	return $DATA . "/pub/model/${model_id}-${color_name}/big_$num.jpg";
}

sub _preview
{
	my ($model_id, $color_name, $num) = @_;
	return $DATA . "/pub/model/${model_id}-${color_name}/preview_$num.jpg";
}

sub add_image
{
	my $self = shift;
	my $image_file = shift;
	my $no = $self->{'num_images'};

	my $model_id = $self->model_id;
	my $color = $self->color;
	my $big = _big($model_id, $color->name, $no);
	my $preview = _preview($model_id, $color->name, $no);

	mkdir("$DATA/pub/model/${model_id}-" . $color->name);

	system("convert '$image_file' -resize x1100 '$big'");
	die "could not convert to big image: $?" if $?;

	system("convert '$image_file' -resize 147x220^ -gravity center -extent 147x220 '$preview'");
	die "could not convert to preview image: $?" if $?;

	$self->{'num_images'} += 1;
	$self->save();
}

sub remove_image
{
	my $self = shift;
	my $num = shift;
	$num += 0;
	die "Invalid image number" if $num >= $self->num_images;

	my $model_id = $self->model_id;
	my $color_name = $self->color->name;
	my $count = $self->num_images;

	my $big = _big($model_id, $color_name, $num);
	my $preview = _preview($model_id, $color_name, $num);

	unlink($big);
	unlink($preview);

	for (my $i = $num; $i < $count-1; $i++)
	{
		my $next_big = _big($model_id, $color_name, $i+1);
		my $next_preview = _preview($model_id, $color_name, $i+1);

		rename($next_big, $big);
		rename($next_preview, $preview);

		$big = $next_big;
		$preview = $next_preview;
	}

	$self->{'num_images'} -= 1;
	$self->save();
}

1;
