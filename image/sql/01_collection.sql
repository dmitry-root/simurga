
set autocommit = 0;

begin;


create table if not exists `material` (
	`id` int unsigned not null auto_increment primary key,
	`name` varchar(32) unique key not null,
	`desc_ru` varchar(256) not null default '',
	`desc_en` varchar(256) not null default ''
) character set 'utf8';


create table if not exists `color` (
	`id` int unsigned not null auto_increment primary key,
	`name` varchar(32) unique key not null,
	`code` char(6) not null,
	`desc_ru` varchar(256) not null default '',
	`desc_en` varchar(256) not null default ''
) character set 'utf8';


create table if not exists `model` (
	`id` int unsigned not null primary key,
	`short_title_ru` varchar(256),
	`short_title_en` varchar(256),
	`title_ru` varchar(256) not null default '',
	`title_en` varchar(256) not null default ''
) character set 'utf8';


create table if not exists `model_color` (
	`id` int unsigned not null auto_increment primary key,
	`model_id` int unsigned not null,
	`color_id` int unsigned not null,
	`num_images` int unsigned not null default 0,
	`sizes` int unsigned not null default 0,
	`material_id` int unsigned,
	`price_ru` decimal(10, 2) not null default 0,
	`price_en` decimal(10, 2) not null default 0,
	`desc_ru` text not null default '',
	`desc_en` text not null default '',

	foreign key `model_fk` (`model_id`) references `model` (`id`) on delete cascade on update cascade,
	foreign key `color_fk` (`color_id`) references `color` (`id`) on delete cascade on update cascade,
	foreign key `material_fk` (`material_id`) references `material` (`id`) on delete set null on update cascade,
	index `model_idx` (`model_id`)
) character set 'utf8';


create or replace view `collection_view_ru` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		if(`model`.`short_title_ru` is null, `model`.`title_ru`, `model`.`short_title_ru`) as `title`,
		cast(`model_color`.`price_ru` * 100 as unsigned int) as `price`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
	order by `model_color`.`id` desc;

create or replace view `collection_view_en` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		if(`model`.`short_title_en` is null, `model`.`title_en`, `model`.`short_title_en`) as `title`,
		cast(`model_color`.`price_en` * 100 as unsigned int) as `price`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
	order by `model_color`.`id` desc;


create or replace view `model_view_ru` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		`color`.`desc_ru` as `color_desc`,
		`model`.`title_ru` as `model_title`,
		`model_color`.`num_images` as `num_images`,
		`model_color`.`sizes` as `sizes`,
		`material`.`desc_ru` as `material_desc`,
		cast(`model_color`.`price_ru` * 100 as unsigned int) as `price`,
		`model_color`.`desc_ru` as `desc`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
		left join `material` on (`model_color`.`material_id` = `material`.`id`);

create or replace view `model_view_en` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		`color`.`desc_en` as `color_desc`,
		`model`.`title_en` as `model_title`,
		`model_color`.`num_images` as `num_images`,
		`model_color`.`sizes` as `sizes`,
		`material`.`desc_en` as `material_desc`,
		cast(`model_color`.`price_en` * 100 as unsigned int) as `price`,
		`model_color`.`desc_en` as `desc`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
		left join `material` on (`model_color`.`material_id` = `material`.`id`);

commit;
