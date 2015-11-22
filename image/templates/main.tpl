<!DOCTYPE html>
<html>
	<head>
		<base href="<%base_uri%>" />
		<meta charset="UTF-8" />
		<title><%title%> &mdash; Simurga</title>
		<link rel="stylesheet" href="/css/slippry.css" type="text/css" />
		<link rel="stylesheet" href="/style.css" type="text/css" />
		<script src="/js/jquery.min.1.10.2.js" type="text/javascript"></script>
		<script src="/js/slippry.min.js" type="text/javascript"></script>
		<script src="/script.js" type="text/javascript"></script>
	</head>
	<body class="horz">
		<div class="navigation">
			<div class="logo">
				<img src="/logo.png" width="128" height="132" alt="Simurga logo" />
			</div>
			<nav>
				<ul>
					<li class="<%lookbook_active%>"><a href="/lookbook">Lookbook</a></li>
					<li class="<%collection_active%>"><a href="/collection">Collection</a></li>
					<li class="<%phylosophy_active%>"><a href="/phylosophy">Phylosophy</a></li>
					<li class="<%contacts_active%>"><a href="/contacts">Contacts</a></li>
				</ul>
			</nav>
		</div>
		<article>
<%body%>
		</article>
	</body>
</html>
