			<div class="model-photo-wrapper">
				<div class="photo">
					<a class="fancybox" rel="group" href="<%default_image%>">
						<img src="<%default_image%>" alt="Model photo" />
					</a>
				</div>
				<div class="previews">
<%#for previews%>
					<a class="fancybox" rel="group" href="<%big%>"><img src="<%preview%>" alt="Preview <%index%>" class="preview<%index%>" /></a>
<%#end%>
				</div>
			</div>
			<div class="model-description">
				<h1><%model_title%></h1>
				<p class="serial"><strong>Номер модели</strong>: <%model_id%></p>
				<p class="color"><strong>Цвет</strong>: <%color_desc%></p>
				<p class="size"><strong>Размер</strong>: <%sizes%></p>
				<p class="formula"><strong>Состав</strong>: <%material_desc%></p>
				<div>
					<%desc%>
					<p>Купить одежду для йоги в Новосибирске можно в нашем Интернет-магазине.</p>
					<p>Доставка по России и всему миру.</p>
					<p>В Новосибирске работает курьер: Вы можете заказать до 10 моделей одежды на примерку, и выбрать то, что Вам подойдет.</p>
				</div>
				<p class="price"><strong>Цена</strong>:
					<%#if special_price%><span class="strike"><%price%> руб.</span> <span class="specialprice"><%special_price%> руб.</span><%#else%><%price%> руб.<%#end%>
				</p>
				<p class="buy"><a href="/howtobuy">Купить</a></p>
			</div>
