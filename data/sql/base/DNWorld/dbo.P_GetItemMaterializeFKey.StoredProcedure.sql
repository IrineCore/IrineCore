USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetItemMaterializeFKey]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-04
description : ItemMaterializeFKey ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetItemMaterializeFKey]
	  @inbItemSerial bigint --// ??? ?? ??
	, @inyItemMaterializeCode tinyint OUTPUT --// 1=??, 2=?? ??, 3=?????, 4=??, 5=?????? ??, 6=?? ??, 7=?? ??, 8=DNA??? ??, 9=???? ??, 10=???  ?? ??, 11=??? ???? ??, 13=??, 14=??, 15=?? ??, 16=PVP ??, 17=?? ??, 18=??? ?? ??, 19=Fixed Item, 20=??????, 21=??? ??
	, @inbItemMaterializeFKey bigint OUTPUT --// ItemMaterializeCode ? ?? ? ?. 1:PartyID or NULL, 2:MissionID, 3:QuestID, 4:PurchaseID, 5:NPCID, 6/7/8:PurchaseID, 10:MailID, 11:NPCID, 13:NULL, 14:NULL, 15:NULL, 16:NULL, 17:NULL, 18:MailID, 19:ItemSerial
--WITH EXECUTE AS 'SPExecutor'
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
	SELECT @inyItemMaterializeCode = ItemMaterializeCode
		, @inbItemMaterializeFKey = ItemMaterializeFKey
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END
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
