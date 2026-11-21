USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPetalIncome]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPetalIncome]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPetalIncome] AS' 
END
GO
/**
version : 18
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-04
description : ??? ?????. (?? ???, ??? ??? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101178 = @inbPurchaseID ?? ???? ????.
history :
3 = @inbPurchaseID ?? ?? ???? ?? ??? ????? ?? at 2010-11-10
4 = input param. @inbPurchaseOrderDetailID ?? at 2010-11-19
5 = input param. @inbPurchaseOrderDetailID ?? at 2010-11-22
6 = input param. @vchPetalVoucherIDs ?? at 2010-11-22
7 = input param. @vchPetalVoucherIDs ?? & PurchaseOrderID? uniqueness ?? at 2010-11-25
8 = ??? ??? ??? ?? ?? ?? ?? ?? (input param. @intTradeID ??) at 2011-01-24
9 = input param. @intTradeID ?? at 2011-01-24
10 = PurchaseOrderID ?? ?? NULL? INSERT?? ?? ?? at 2011-05-23
11 = @inyPetalIncomeCode=8 ??     by ??? at 2012-01-25
12 = input param. @bitCheckPurchaseID ?? by ??? at 2012-09-27
14 = input param. @bitCheckPurchaseID ??, @inbPurchaseID => @inbItemSerial? ?? by ??? at 2012-09-27
15 = input param. @inyPetalIncomeCode = 6 & @inbItemSerial is null ?? 101178 ????? ?? by ??? at 2012-11-19
15 = ?? ?? ?? input param. @inyPetalIncomeCode = 12 ???? @inbGambelID ?? by ??? at 2013-01-24
**/

ALTER PROCEDURE [dbo].[P_AddPetalIncome]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// @inyPetalIncomeCode ?? 6? ???? ?? (?? ???? ??? ??? ?)
	, @intPetalAmount int --// ??? ?? ??
	, @intPetalBalance int OUTPUT --// ?? ? ?? ??
	, @inyPetalIncomeCode tinyint = 6 --// 6=?? ???, 7=??? ??? ?? ??, 8=(DOORS) ?? ?? ??, 12=??????, 13=???????, 14=???????
	, @inbGambleID bigint = NULL --//@inyPetalIncomeCode ?? 12, 13? ???? ?? (??ID)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?? ????? 101178? ?????.*/
	IF @inyPetalIncomeCode = 6 AND (@inbItemSerial IS NULL OR EXISTS (
		SELECT *
		FROM dbo.PetalIncome WITH (NOLOCK, INDEX(IX_UNF_PetalIncome_ItemSerial), FORCESEEK)
		WHERE PetalIncomeCode = 6 AND ItemSerial IS NOT NULL AND ItemSerial = @inbItemSerial
	
	))
	BEGIN
		SET @intReturnValue = 101178;
		GOTO ErrorHandler;
	END
	/**_# ???? AccountID? ?????.*/
	SELECT @intAccountID = AccountID
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	/**_## ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# [PetalIncome] ???? INSERT???.*/
	/**_# ???? ?? PurchaseOrderID?? 101178? ?????.*/
	INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PetalAmount, IncomeDate, ItemSerial)
	VALUES (@intAccountID, @inyPetalIncomeCode, @intPetalAmount, GETDATE(), CASE WHEN @inyPetalIncomeCode = 6 THEN @inbItemSerial WHEN @inyPetalIncomeCode IN (12, 13) THEN @inbGambleID ELSE NULL END);

	/**_# ??? ?? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_GetPetalBalance
		  @intAccountID = @intAccountID
		, @intPetalBalance = @intPetalBalance OUTPUT;
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
