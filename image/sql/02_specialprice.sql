
alter table `model_color`
	add column `special_price_ru` decimal(10, 2) null default null after `price_en`;

alter table `model_color`
	add column `special_price_en` decimal(10, 2) null default null after `special_price_ru`;

create or replace view `collection_view_ru` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		if(`model`.`short_title_ru` is null, `model`.`title_ru`, `model`.`short_title_ru`) as `title`,
		cast(`model_color`.`price_ru` * 100 as unsigned int) as `price`,
		if(`model_color`.`special_price_ru` is null, null, cast(`model_color`.`special_price_ru` * 100 as unsigned int)) as `special_price`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
	order by `model_color`.`id` desc;

create or replace view `collection_view_en` as
	select
		`model`.`id` as `model_id`,
		`color`.`name` as `color_name`,
		if(`model`.`short_title_en` is null, `model`.`title_en`, `model`.`short_title_en`) as `title`,
		cast(`model_color`.`price_en` * 100 as unsigned int) as `price`,
		if(`model_color`.`special_price_ru` is null, null, cast(`model_color`.`special_price_ru` * 100 as unsigned int)) as `special_price`
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
		if(`model_color`.`special_price_ru` is null, null, cast(`model_color`.`special_price_ru` * 100 as unsigned int)) as `special_price`,
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
		if(`model_color`.`special_price_ru` is null, null, cast(`model_color`.`special_price_ru` * 100 as unsigned int)) as `special_price`,
		`model_color`.`desc_en` as `desc`
	from `model_color`
		inner join `model` on (`model`.`id` = `model_color`.`model_id`)
		inner join `color` on (`model_color`.`color_id` = `color`.`id`)
		left join `material` on (`model_color`.`material_id` = `material`.`id`);

