USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_HashPassphrase2]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[FN_HashPassphrase2]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'CREATE FUNCTION [dbo].[FN_HashPassphrase2](
	@vchPassphrase varchar(12),
	@intAccountID int
)
RETURNS binary(20)
AS
BEGIN
	RETURN (
		HASHBYTES(''md5'',@vchPassphrase)
	);
END
' 
END
GO
