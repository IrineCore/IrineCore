USE [dnmembership]
GO
/****** Object:  Table [dbo].[BlockedIPExceptions]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BlockedIPExceptions]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BlockedIPExceptions](
	[StartIP] [int] NOT NULL,
	[EndIP] [int] NOT NULL,
	[Description] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AdminID] [int] NOT NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_BlockedIPExceptions] PRIMARY KEY CLUSTERED 
(
	[StartIP] ASC,
	[EndIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
