USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFairy]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-24
description : ??(???)? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103395 = ?? ?? ???? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddFairy]
	  @inbFairyItemSerial bigint --// ?? ??? ???
	, @inyFairyLevel tinyint --// ?? ??
	, @intFairyExp int --// ?? ???
	, @intFairyStatus1 int = NULL --// ????1
	, @intFairyStatus2 int = NULL --// ????2
	, @intFairyStatus3 int = NULL --// ????3
	, @intFairyStatus4 int = NULL --// ????4
	, @intFairyStatus5 int = NULL --// ????5
	, @intFairyStatus6 int = NULL --// ????6
	, @inbOwnerCharacterID bigint --// ?? ??? ID
	, @inyShapeID tinyint --// ?? ShapeID
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
	/**_# [Fairies] ???? INSERT???.*/
	INSERT dbo.Fairies (FairyItemSerial, FairyLevel, FairyExp, FairyStatus1, FairyStatus2, FairyStatus3, FairyStatus4, FairyStatus5, FairyStatus6, OwnerCharacterID, ShapeID)
	VALUES (@inbFairyItemSerial, @inyFairyLevel, @intFairyExp, @intFairyStatus1, @intFairyStatus2, @intFairyStatus3, @intFairyStatus4, @intFairyStatus5, @intFairyStatus6, @inbOwnerCharacterID, @inyShapeID);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 103395;	

	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
