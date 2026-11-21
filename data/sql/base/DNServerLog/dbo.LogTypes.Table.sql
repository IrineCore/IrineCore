USE [dnserverlog]
GO
/****** Object:  Table [dbo].[LogTypes]    Script Date: 11/14/2026 8:57:59 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[LogTypes](
	[LogType] [tinyint] NOT NULL,
	[LogTypeName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[HideFlag] [bit] NOT NULL,
 CONSTRAINT [PK_LogTypes] PRIMARY KEY CLUSTERED 
(
	[LogType] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[LogTypes] ADD  CONSTRAINT [DF_LogTypes_HideFlag]  DEFAULT ((0)) FOR [HideFlag]
GO
