USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddBlockedDNGirl]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-02
description : ??? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103414 = ?? ??? ??????.
**/

CREATE PROCEDURE [dbo].[P_DLS_AddBlockedDNGirl]
	  @inbDNGirlCharacterID bigint --// ??? ??? ID
	, @nvcBlockedReason nvarchar(100) --// ?? ??(100?)
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
	BEGIN TRANSACTION;
	
	/**_# [BlockedDNGirls] ???? INSERT???.*/
	INSERT dbo.BlockedDNGirls (DNGirlCharacterID, BlockedReason, BlockedDate)
	VALUES (@inbDNGirlCharacterID, @nvcBlockedReason, GETDATE());
	
	/**_# ??? ???? ? & ??? ???? ??? ???.*/
	UPDATE dbo.DNGirlIntimacy
	SET Intimacy = 0, FanFlag = 0
	WHERE DNGirlCharacterID = @inbDNGirlCharacterID;
	
	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 103414;
	
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
