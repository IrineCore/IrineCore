USE [dnmembership]
GO
/****** Object:  Table [dbo].[SimpleConfigValues]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SimpleConfigValues]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[SimpleConfigValues](
	[SimpleConfigCode] [tinyint] NOT NULL,
	[SimpleConfigValue] [int] NOT NULL,
 CONSTRAINT [PK_SimpleConfigValues] PRIMARY KEY CLUSTERED 
(
	[SimpleConfigCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
