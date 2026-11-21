USE [dnstaging]
GO
/****** Object:  Table [dbo].[IndexUsages]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[IndexUsages](
	[BasisDate] [date] NOT NULL,
	[ServerName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[DatabaseName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[TableName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[IndexName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[UserScans] [bigint] NULL,
	[UserSeeks] [bigint] NULL,
	[UserLookups] [bigint] NULL,
	[UserUpdates] [bigint] NULL,
 CONSTRAINT [PK_IndexUsages] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[ServerName] ASC,
	[DatabaseName] ASC,
	[TableName] ASC,
	[IndexName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
