USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_HashPassphrase]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[FN_HashPassphrase]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ????? Hash???.
**/

CREATE FUNCTION [dbo].[FN_HashPassphrase](
	@vchPassphrase varchar(12),
	@intAccountID int
)
RETURNS binary(20)
AS
BEGIN
	RETURN (
		HASHBYTES(''SHA1'', ''818719D11B0B41CA80C22F5D9901E92B'' + STR(@intAccountID, 10) + @vchPassphrase)
	);
END
' 
END
GO
