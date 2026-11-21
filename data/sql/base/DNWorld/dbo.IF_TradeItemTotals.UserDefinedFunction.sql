USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[IF_TradeItemTotals]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-09-17
description : 
**/

CREATE FUNCTION [dbo].[IF_TradeItemTotals] (
	  @intItemID int --// ??? ID
	, @inyItemLevel tinyint --// ?? ??
	, @intListPrice int --// ??
	, @bitNoticeFlag bit --// ?? ?? ??
)
RETURNS table
AS
RETURN
	SELECT COUNT(*) AS TradeCount
		, ISNULL(MIN(TR.ListPrice), 0) AS MinPrice
	FROM dbo.Trade TR WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
	WHERE TR.TradeStatusCode = 0
		AND TR.ItemID = @intItemID
		AND TR.TradeExpireDate >= GETDATE()
		AND (@bitNoticeFlag = 0 OR (@bitNoticeFlag = 1 AND TR.ItemLevel >= @inyItemLevel AND TR.ListPrice <= @intListPrice));
GO
