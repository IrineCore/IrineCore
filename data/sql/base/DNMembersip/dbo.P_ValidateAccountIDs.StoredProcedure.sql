USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ValidateAccountIDs]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ValidateAccountIDs]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ValidateAccountIDs] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ?? ID? ???? ?? - ?? ??? ?? - ???. (??? ?? 1,000? ?)
result set : AccountID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_ValidateAccountIDs]
	  @vchAccountIDs  varchar(max) --// ,? ?????? AccountID ??? (?? 1,000?)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblAccountIDs table (AccountID int NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblAccountIDs (AccountID)
	SELECT CAST(string AS int)
	FROM dbo.TF_Split(@vchAccountIDs, ',')
	WHERE string > ''
	GROUP BY string
	ORDER BY string;

	SELECT T.AccountID
	FROM @tblAccountIDs T
	WHERE NOT EXISTS (
		SELECT * FROM dbo.Accounts WITH (NOLOCK) WHERE AccountID = T.AccountID
	)
	ORDER BY AccountID;
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
