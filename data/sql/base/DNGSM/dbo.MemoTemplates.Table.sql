USE [dngsm]
GO
/****** Object:  Table [dbo].[MemoTemplates]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MemoTemplates](
	[MemoTemplateID] [int] IDENTITY(1,1) NOT NULL,
	[RepositoryName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[MethodName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[MemoSubject] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[MemoContent] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[AdminID] [int] NOT NULL,
 CONSTRAINT [PK_MemoTemplates] PRIMARY KEY CLUSTERED 
(
	[MemoTemplateID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_MemoTemplates_AdminID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MemoTemplates_AdminID] ON [dbo].[MemoTemplates]
(
	[AdminID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
