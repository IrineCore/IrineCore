USE [dnmembership]
GO
/****** Object:  Table [dbo].[Codes]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Codes]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Codes](
	[codeColumnName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[codeSN] [int] NOT NULL,
	[description] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[creator] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[createdDate] [datetime] NOT NULL,
 CONSTRAINT [PK_Codes] PRIMARY KEY CLUSTERED 
(
	[codeColumnName] ASC,
	[codeSN] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_Codes_codeColumnName]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Codes]') AND name = N'IX_NN_Codes_codeColumnName')
CREATE NONCLUSTERED INDEX [IX_NN_Codes_codeColumnName] ON [dbo].[Codes]
(
	[codeColumnName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CodeKinds_Codes_codeColumnName]') AND parent_object_id = OBJECT_ID(N'[dbo].[Codes]'))
ALTER TABLE [dbo].[Codes]  WITH CHECK ADD  CONSTRAINT [FK_CodeKinds_Codes_codeColumnName] FOREIGN KEY([codeColumnName])
REFERENCES [dbo].[CodeKinds] ([codeColumnName])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CodeKinds_Codes_codeColumnName]') AND parent_object_id = OBJECT_ID(N'[dbo].[Codes]'))
ALTER TABLE [dbo].[Codes] CHECK CONSTRAINT [FK_CodeKinds_Codes_codeColumnName]
GO
