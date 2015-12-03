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
				<div><%desc%></div>
				<p class="price"><strong>Цена</strong>:
					<%#if special_price%><span class="strike"><%price%> руб.</span> <span class="specialprice"><%special_price%> руб.</span><%#else%><%price%> руб.<%#end%>
				</p>
				<p class="buy"><a href="/contacts">Купить</a></p>
			</div>
