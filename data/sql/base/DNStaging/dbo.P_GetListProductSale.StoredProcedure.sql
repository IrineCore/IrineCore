USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListProductSale]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-19
description : ?? ??? ?? ??? ?????.
result set : BasisDate, UniqueBuyerCount, UniqueCharacterCount, SaleCount, SaleAmount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListProductSale]
	  @intProductID int --// ?? ID
	, @datStartDate date --// ?? ?? ??
	, @datEndDate date --// ?? ? ??
	, @inyPayMethodCode tinyint --// 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??
	, @inyPetalIncomeCode tinyint --// 0=??????, 1=??, 2=??? ??, 3=??? ?? ??, 4=???, 5=??? ??? ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT S.BasisDate, S.UniqueBuyerCount, S.UniqueCharacterCount, S.SaleCount, S.SaleAmount
	FROM dbo.ST_DailySalesByProduct S WITH (NOLOCK)
	WHERE BasisDate BETWEEN @datStartDate AND @datEndDate AND S.ProductID = @intProductID
		AND S.PayMethodCode = @inyPayMethodCode AND S.PetalIncomeCode = COALESCE(@inyPetalIncomeCode, 0)
	ORDER BY S.BasisDate;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
