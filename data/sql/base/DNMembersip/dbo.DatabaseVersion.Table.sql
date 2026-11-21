USE [dnmembership]
GO
/****** Object:  Table [dbo].[DatabaseVersion]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DatabaseVersion]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[DatabaseVersion](
	[version] [int] NOT NULL,
	[registerDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DatabaseVersion] PRIMARY KEY CLUSTERED 
(
	[version] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_DatabaseVersion]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[DatabaseVersion] ADD  CONSTRAINT [DF_DatabaseVersion]  DEFAULT (getdate()) FOR [registerDate]
END
GO
