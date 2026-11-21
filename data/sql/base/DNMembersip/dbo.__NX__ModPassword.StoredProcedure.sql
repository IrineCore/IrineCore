USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[__NX__ModPassword]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[__NX__ModPassword]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[__NX__ModPassword] AS' 
END
GO
/**
0 SUCCESS
1 NOT EXIST
2 UNKNOWN ERROR
**/

ALTER PROCEDURE [dbo].[__NX__ModPassword]
	  @AccountName varchar(18)  -- ????6~18?
	, @NxLoginPwd varchar(18)-- ??,????6~18?
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
DECLARE @NxLoginPwdMD5 varchar(32);
DECLARE @intAccountID int = 0

	SELECT @intAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) 
		WHERE AccountName = @AccountName

	/** Account not exist.*/
	IF @intAccountID = 0
	BEGIN
		RETURN 1
	END
	SELECT @NxLoginPwdMD5 = UPPER(substring(sys.fn_VarBinToHexStr(hashbytes('MD5', @NxLoginPwd)),3,32));

	UPDATE dbo.Accounts SET NxLoginPwd = @NxLoginPwdMD5 WHERE AccountID = @intAccountID;

	IF(@@ERROR=0)
		RETURN 0;
	ELSE
		RETURN 2;
GO
