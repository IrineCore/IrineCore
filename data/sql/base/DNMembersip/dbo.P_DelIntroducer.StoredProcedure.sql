USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DelIntroducer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DelIntroducer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DelIntroducer] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-01
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DelIntroducer]
	  @nvcAccountName nvarchar(30) --// ??? ?? ??
	, @nvcIntroducedAccountName nvarchar(30) --// ???? ?? ??? ?? ??
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
	/**_# [Introducers] ????? DELETE???.*/
	DELETE dbo.Introducers
	FROM dbo.Introducers A
		INNER JOIN dbo.Accounts B WITH (NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.Accounts C WITH (NOLOCK) ON A.IntroducedAccountID = C.AccountID
	WHERE B.AccountName = @nvcAccountName AND C.AccountName = @nvcIntroducedAccountName;
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
