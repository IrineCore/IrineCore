USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddBlockedIPException]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddBlockedIPException]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddBlockedIPException] AS' 
END
GO
/**
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ???? ?? IP??? ?????.
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
101103 = ?? ??? ?? ?? IP??? ?? ??? ?????.
**/

ALTER PROCEDURE [dbo].[P_AddBlockedIPException]
	  @vchStartIP varchar(15) --// ?? ?? IP??? ?? IP ??
	, @vchEndIP varchar(15) --// ?? ?? IP??? ? IP ??
	, @nvcDescription nvarchar(300) = NULL --// (optional) IP??? ?? ??
	, @intAdminID int --// ?? ?? IP??? ??? ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@intStartIP int,
	@intEndIP int,
	@intStartIPTemp int,
	@intEndIPTemp int,
	@i int,
	@j int;

DECLARE @tblDuplicatedIPBands table (
	seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	StartIP int NOT NULL,
	EndIP int NOT NULL,
	BlockedIPCode tinyint NOT NULL,
	[Description] nvarchar(100) NULL,
	AdminID nvarchar(50) NOT NULL,
	RegisterDate datetime NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 4010001;
	GOTO ErrorHandler;
END;

BEGIN TRY
	SET @intStartIP = dbo.FN_IP2IPNumber(@vchStartIP);
	SET @intEndIP = dbo.FN_IP2IPNumber(@vchEndIP);

	/**_# ?? ??? ?? ?? IP??? ?? ??? ????, 101103? ?????.*/
	IF EXISTS (
		SELECT *
		FROM dbo.BlockedIPExceptions WITH (NOLOCK)
		WHERE StartIP BETWEEN (SELECT @intStartIP) AND (SELECT @intEndIP)
			OR EndIP BETWEEN (SELECT @intStartIP) AND (SELECT @intEndIP)
			OR @intStartIP BETWEEN StartIP AND EndIP
			OR @intEndIP BETWEEN StartIP AND EndIP
	)
	BEGIN
		SET @intReturnValue = 101103;
		GOTO ErrorHandler;
	END;

	INSERT @tblDuplicatedIPBands (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate)
	SELECT StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate
	FROM dbo.BlockedIPs WITH (NOLOCK)
	WHERE StartIP BETWEEN (SELECT @intStartIP) AND (SELECT @intEndIP)
		OR EndIP BETWEEN (SELECT @intStartIP) AND (SELECT @intEndIP)
		OR @intStartIP BETWEEN StartIP AND EndIP
		OR @intEndIP BETWEEN StartIP AND EndIP;

	SELECT @i = 1, @j = @@ROWCOUNT;

	/**_# ????? ?? ?? IP? ?? ?? IP ??? ?????...*/
	/**_## ?? ?? IP? ???? ?? ?? IP ??? ?????.*/
	IF @j > 0
		DELETE dbo.BlockedIPs
		FROM dbo.BlockedIPs BIP
			INNER JOIN @tblDuplicatedIPBands T ON BIP.StartIP = T.StartIP AND BIP.EndIP = T.EndIP;

	/**_## ??? ?? IP ???? ?? ?? IP? ??? ??? [BlockedIPs] ???? INSERT ???.*/
	WHILE @i <= @j
	BEGIN
		SELECT @intStartIPTemp = StartIP, @intEndIPTemp = EndIP
		FROM @tblDuplicatedIPBands
		WHERE seq = @i;

		IF @intStartIP > @intStartIPTemp AND @intStartIP <= @intEndIPTemp AND @intEndIP >= @intEndIPTemp
			INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate)
			SELECT StartIP, (@intStartIP - 1), BlockedIPCode, [Description], AdminID, RegisterDate
			FROM @tblDuplicatedIPBands
			WHERE seq = @i;

		ELSE IF @intEndIP >= @intStartIPTemp AND @intEndIP < @intEndIPTemp AND @intStartIP <= @intStartIPTemp
			INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate)
			SELECT (@intEndIP + 1), EndIP, BlockedIPCode, [Description], AdminID, RegisterDate
			FROM @tblDuplicatedIPBands
			WHERE seq = @i;
		ELSE IF (@intStartIP BETWEEN @intStartIPTemp AND @intEndIPTemp) AND (@intEndIP BETWEEN @intStartIPTemp AND @intEndIPTemp)
		BEGIN
			INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate)
			SELECT StartIP, (@intStartIP - 1), BlockedIPCode, [Description], AdminID, RegisterDate
			FROM @tblDuplicatedIPBands
			WHERE seq = @i AND StartIP < @intStartIP;

			INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate)
			SELECT (@intEndIP + 1), EndIP, BlockedIPCode, [Description], AdminID, RegisterDate
			FROM @tblDuplicatedIPBands
			WHERE seq = @i AND EndIP > @intEndIP;
		END;

		SET @i = @i + 1;
	END;

	/**_# [BlockedIPExceptions] ???? INSERT???.*/
	INSERT dbo.BlockedIPExceptions (StartIP, EndIP, [Description], AdminID, RegisterDate)
	VALUES (@intStartIP, @intEndIP, @nvcDescription, @intAdminID, GETDATE());
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
