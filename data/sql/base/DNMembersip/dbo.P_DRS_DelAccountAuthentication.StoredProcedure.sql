USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_DelAccountAuthentication]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_DelAccountAuthentication]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_DelAccountAuthentication] AS' 
END
GO
/**
version : 10
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-09
description : ????? ???? ???? ?? ??? ID? ?????.
result set : WorldID / int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ??? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_DelAccountAuthentication]
	  @intAccountID int --// ??ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
        , @dt2Now datetime2(0) = GETDATE()
        , @vchCharacterIDs varchar(max) = '';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ??? ???? ?????.*/
	SELECT @vchCharacterIDs += CAST(CharacterID AS varchar(10)) + ','
	FROM dbo.Characters WITH (NOLOCK)
	WHERE AccountID = @intAccountID AND DeleteFlag = 0;

	SET @vchCharacterIDs = SUBSTRING(@vchCharacterIDs, 0, LEN(@vchCharacterIDs) - 1);
	
	/**_# [MobileAuthentication] ???? DELETE???.*/
	DELETE dbo.MobileAuthentication 
	OUTPUT deleted.AccountID, @dt2Now, @vchCharacterIDs
	INTO dbo.AccountAuthenticationDelQueue (AccountID, RegistedDate, CharacterList)
	WHERE AccountID = @intAccountID;		

	/**_# ???? ?? ??? ID? ?????.*/
	--SELECT DISTINCT WorldID
	--FROM dbo.Characters WITH (NOLOCK)
	--WHERE AccountID = @intAccountID;

	/**_# ??? ?? ?? ?? */
	EXEC dbo.P_AddMobileAuthenticationLog
			@dt2LogDate = @dt2Now
		  , @intAccountID = @intAccountID
		  , @inyAuthenticationLogCode = 2
		  , @intAccountAuthenticationKey = NULL;

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
