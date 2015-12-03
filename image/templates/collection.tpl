<%#for collection%>
			<div class="collection">
				<a href="/model/<%model_id%>-<%color_name%>">
					<img src="/model/<%model_id%>-<%color_name%>/preview_0.jpg" width="147" height="220" alt="" />
					<div class="title"><%title%></div>
					<div class="price<%#if special_price%> strike<%#end%>"><%price%> руб.</div>
				</a>
				<%#if special_price%><div class="specialprice"><%special_price%></div><%#end%>
			</div>
<%#end%>
